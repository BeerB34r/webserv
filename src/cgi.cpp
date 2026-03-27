/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   cgi.cpp                                                 :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/12 22:11:37 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/12 22:31:45 by mde-beer            ########   odam.nl   */
/*                                                                            */
/*   —————No norm compliance?——————                                           */
/*   ⠀⣞⢽⢪⢣⢣⢣⢫⡺⡵⣝⡮⣗⢷⢽⢽⢽⣮⡷⡽⣜⣜⢮⢺⣜⢷⢽⢝⡽⣝                                           */
/*   ⠸⡸⠜⠕⠕⠁⢁⢇⢏⢽⢺⣪⡳⡝⣎⣏⢯⢞⡿⣟⣷⣳⢯⡷⣽⢽⢯⣳⣫⠇                                           */
/*   ⠀⠀⢀⢀⢄⢬⢪⡪⡎⣆⡈⠚⠜⠕⠇⠗⠝⢕⢯⢫⣞⣯⣿⣻⡽⣏⢗⣗⠏⠀                                           */
/*   ⠀⠪⡪⡪⣪⢪⢺⢸⢢⢓⢆⢤⢀⠀⠀⠀⠀⠈⢊⢞⡾⣿⡯⣏⢮⠷⠁⠀⠀⠀                                           */
/*   ⠀⠀⠀⠈⠊⠆⡃⠕⢕⢇⢇⢇⢇⢇⢏⢎⢎⢆⢄⠀⢑⣽⣿⢝⠲⠉⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⠀⡿⠂⠠⠀⡇⢇⠕⢈⣀⠀⠁⠡⠣⡣⡫⣂⣿⠯⢪⠰⠂⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⡦⡙⡂⢀⢤⢣⠣⡈⣾⡃⠠⠄⠀⡄⢱⣌⣶⢏⢊⠂⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⢝⡲⣜⡮⡏⢎⢌⢂⠙⠢⠐⢀⢘⢵⣽⣿⡿⠁⠁⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⠨⣺⡺⡕⡕⡱⡑⡆⡕⡅⡕⡜⡼⢽⡻⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⣼⣳⣫⣾⣵⣗⡵⡱⡡⢣⢑⢕⢜⢕⡝⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⣴⣿⣾⣿⣿⣿⡿⡽⡑⢌⠪⡢⡣⣣⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⡟⡾⣿⢿⢿⢵⣽⣾⣼⣘⢸⢸⣞⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⠁⠇⠡⠩⡫⢿⣝⡻⡮⣒⢽⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ——————————————————————————————                                           */
/* ************************************************************************** */

#include <defaultpage.hpp>
#include <cgi.hpp>
#include <debug.hpp>
#include <HTTPparsing.hpp>
#include <sys/wait.h> // waitpid
#include <fcntl.h> // pipe2
#include <filesystem>
#include <string>

#define BUFFER_SIZE 1024

using namespace std::literals;

static inline auto	createExecveArg(std::vector<std::string> v) -> std::unique_ptr<char*[]>{;
	std::unique_ptr<char*[]>	rv(new char*[v.size() + 1]);

	int i = 0;
	for (std::string& s : v) rv[i++] = s.data();
	rv[i] = NULL;
	return rv;
}

static inline auto	extensionArgs(const std::filesystem::path& bin) -> std::vector<std::string> {
	// indefinite is such a fuckass word. because of it, now we use the timeout
	// POSIX utility to make sure that theres a time limit on any CGI request
	// (the ones that can take time)
	// no operation outside of CGI should ever take enough time for the 30s timeout to matter
	if (!bin.has_extension())
		return {"/usr/bin/env", "-S", "timeout", "-k", std::to_string(TIMEOUT.count()), std::to_string(TIMEOUT.count()), bin};
	if (bin.extension().string() == ".cgi")
		return {"/usr/bin/env", "-S", "timeout", "-k", std::to_string(TIMEOUT.count()), std::to_string(TIMEOUT.count()), bin};
	if (bin.extension().string() == ".hs")
		return {"/usr/bin/env", "-S", "timeout", "-k", std::to_string(TIMEOUT.count()), std::to_string(TIMEOUT.count()), "/usr/bin/env", "-S", "/home/mde-beer/sgoinfre/.ghcup/bin/ghc", "--run", bin}; // normally this would just say /usr/bin/env -S ghc, but since ghc isnt on the system path (i think) it doesnt work
	if (bin.extension().string() == ".py")
		return {"/usr/bin/env", "-S", "timeout", "-k", std::to_string(TIMEOUT.count()), std::to_string(TIMEOUT.count()), "/usr/bin/env", "-S", "python3", bin};
	if (bin.extension().string() == ".sh")
		return {"/usr/bin/env", "-S", "timeout", "-k", std::to_string(TIMEOUT.count()), std::to_string(TIMEOUT.count()), "/usr/bin/env", "-S", "sh", bin};
	return {"/usr/bin/env", "-S", "timeout", "-k", std::to_string(TIMEOUT.count()), std::to_string(TIMEOUT.count()), bin};
}

static inline auto	addrToString(struct in_addr addr) -> std::string {
	short	o1 = (addr.s_addr >> (8 * 0)) & 0xFF;
	short	o2 = (addr.s_addr >> (8 * 1)) & 0xFF;
	short	o3 = (addr.s_addr >> (8 * 2)) & 0xFF;
	short	o4 = (addr.s_addr >> (8 * 3)) & 0xFF;
	return std::to_string(o1) + "." + std::to_string(o2) + "." + std::to_string(o3) + "." + std::to_string(o4);
}

static inline auto	buildEnviron(HTTPMessage& http, const std::string& query, struct in_addr peer_addr, const std::filesystem::path& bin, short port) -> std::vector<std::string> {
	const std::string	defaultEnvs[] = {
		/*"AUTH_TYPE", intentionally omitted, server has no authentication capabilities*/ 
		"CONTENT_LENGTH=" + (http.getBody().empty() ? "" : std::to_string(http.getBody().size())),
		"CONTENT_TYPE=" + (http.getFields().contains("Content-Type") ? http.getFields().at("Content-Type") : ""),
		"GATEWAY_INTERFACE=CGI/1.1", // the rfc used as reference defines cgi/1.1
		/*"PATH_INFO", intentionally omitted, subject states cgi should be based
		 * on file extension, PATH_INFO is only relevant if its based on
		 * directories or similar*/
		/*"PATH_TRANSLATED", see above*/
		"QUERY_STRING=" + query,
		"REMOTE_ADDR=" + addrToString(peer_addr),
		"REMOTE_HOST=" + addrToString(peer_addr), // same as REMOTE_ADDR, cuz i felt like it (and the rfc said i could :P)
		/*"REMOTE_IDENT", chose not to support it */
		/*"REMOTE_USER", authentication not supported*/
		"REQUEST_METHOD=" + toString(std::get<HTTPMessage::RequestData>(http.getData()).method),
		"SCRIPT_NAME=" + bin.string(),
		"SERVER_NAME=" + http.getFields().at("Host"),
		"SERVER_PORT=" + std::to_string(port),
		"SERVER_PROTOCOL=HTTP/1.1",
		"SERVER_SOFTWARE=webserv/1.0",
	};
	std::vector<std::string>	rv;
	for (std::string s : defaultEnvs) rv.push_back(s);
	for (const std::pair<const std::string, std::string> &p : http.getFields()) {
		if (p.first == "Content-Length" || p.first == "Host") continue ;
		if (p.first == "Cookie") rv.push_back("HTTP_COOKIE=" + p.second);
		else rv.push_back(p.first + "=" + p.second);
	}
	return rv;
}

static inline auto	childProcedure [[noreturn]] (const Server& self [[maybe_unused]], HTTPMessage& http, const std::filesystem::path& bin, const std::string& query, struct in_addr peer_addr, int in[2], int out[2]) -> std::pair<int,pid_t> {
	close(in[1]);
	close(out[0]);
	dup2(in[0], STDIN_FILENO);
	dup2(out[1], STDOUT_FILENO);
	execve(extensionArgs(bin).front().c_str(), createExecveArg(extensionArgs(bin)).get(), createExecveArg(buildEnviron(http, query, peer_addr, bin, self.port)).get());
	FATAL("execve failed"); // execve should never fail here
	exit(1);
}

static inline auto	parentProcedure (HTTPMessage http, int in[2], int out[2], pid_t child) -> std::pair<int,pid_t> {
	close(in[0]);
	close(out[1]);
	if (!http.getBody().empty()) write(in[1], http.getBody().c_str(), http.getBody().size());
	close(in[1]);
	return std::make_pair(out[0], child);
}

namespace cgi {
	auto	callback(int output, Server& server, pid_t proc) -> HTTPMessage {
		if (waitpid(proc, NULL, 0) < 0) return server.statusPages.at(500);
		std::string	rv;
		char	buf[BUFFER_SIZE];
		int	bytes;
		do {
			bytes = read(output, buf, BUFFER_SIZE);
			if (bytes < 0) return server.statusPages.at(500);
			rv.append(buf, bytes);
		}	while (bytes != 0);
		close(output);
		Maybe<Pair<std::string,std::vector<std::string>>>	parseRes = (HTTPparsing::fieldLines < HTTPparsing::crlf)(rv); //i hate cpp
		if (!parseRes) return server.statusPages.at(500);
		std::vector<std::string>	fieldlines = parseRes->second;
		std::string	status = "";
		bool	found_status = false, found_length = false;
		for (size_t i = 0; i < fieldlines.size(); ++i) {
			if (fieldlines[i].starts_with("Status:")) {
				status = fieldlines[i].substr(fieldlines[i].find(':') + 1);
				fieldlines.erase(fieldlines.begin() + i);
				if (found_status) return server.statusPages.at(500);
				found_status = true;
			} else if (fieldlines[i].starts_with("Content-Length:")) {
				if (found_length) return server.statusPages.at(500);
				found_length = true;
			}
		}
		std::string	body = parseRes ->first;
		if (!found_length) fieldlines.push_back("Content-Length: " + std::to_string(body.size()));
		if (status.empty()) return HTTPMessage("HTTP/1.1 200 OK", fieldlines, body);
		else return HTTPMessage("HTTP/1.1 " + status, fieldlines, body);
	}
	auto	run(const Server& self, [[maybe_unused]] HTTPMessage http, const std::filesystem::path& bin, const std::string& query, struct in_addr peer_addr) -> std::variant<std::pair<int,pid_t>,HTTPMessage> {
		using fd = int;

		(void)query;
		fd	in[2];
		if (pipe2(in, O_NONBLOCK)) return self.statusPages.at(500); // O_NONBLOCK for whenever i put this in epoll
		fd	out[2];
		if (pipe2(out, O_NONBLOCK)) {
			close(in[0]);
			close(in[1]);
			return self.statusPages.at(500); // O_NONBLOCK for whenever i put this in epoll
		}

		const pid_t	pid = fork();
		if (pid < 0) return self.statusPages.at(500);

		return (pid ? parentProcedure(http, in, out, pid) : childProcedure(self, http, bin, query, peer_addr, in, out));
	}
}
