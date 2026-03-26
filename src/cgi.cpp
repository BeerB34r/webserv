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
	if (!bin.has_extension())
		return {bin};
	if (bin.extension().string() == ".cgi")
		return {bin};
	if (bin.extension().string() == ".hs")
		return {"/usr/bin/env", "-S", "/home/mde-beer/sgoinfre/.ghcup/bin/ghc", "--run", bin}; // normally this would just say /usr/bin/env -S ghc, but since ghc isnt on the system path (i think) it doesnt work
	if (bin.extension().string() == ".py")
		return {"/usr/bin/env", "-S", "python3", bin};
	if (bin.extension().string() == ".sh")
		return {"/usr/bin/env", "-S", "sh", bin};
	return {bin};
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
	return rv;
}

static inline auto	childProcedure [[noreturn]] (const Server& self [[maybe_unused]], HTTPMessage& http, const std::filesystem::path& bin, const std::string& query, struct in_addr peer_addr, int in[2], int out[2]) -> HTTPMessage {
	close(in[1]);
	close(out[0]);
	dup2(in[0], STDIN_FILENO);
	dup2(out[1], STDOUT_FILENO);
	execve(extensionArgs(bin).front().c_str(), createExecveArg(extensionArgs(bin)).get(), createExecveArg(buildEnviron(http, query, peer_addr, bin, self.port)).get());
	FATAL("execve failed"); // execve should never fail here
	exit(1);
}

// TODO: add this to epoll so it isnt blocking
static inline auto	parentProcedure (const Server& self, HTTPMessage& http, int in[2], int out[2], pid_t child) -> HTTPMessage {
	close(in[0]);
	close(out[1]);
	if (!http.getBody().empty()) write(in[1], http.getBody().c_str(), http.getBody().size());
	close(in[1]);
	if (waitpid(child, NULL, 0) < 0) return self.statusPages.at(500);
	std::string	rv;
	char buf[BUFFER_SIZE];
	int	bytes;
	do {
		bytes = read(out[0], buf, BUFFER_SIZE);
		rv.append(buf, bytes);
	} while (bytes != 0);
	close(out[0]);
	Maybe<Pair<std::string,std::vector<std::string>>>	parseRes = (HTTPparsing::fieldLines < HTTPparsing::crlf)(rv); // holy type
	if (!parseRes) return self.statusPages.at(500);
	std::vector<std::string>	fieldlines = parseRes->second;
	std::string	body = parseRes->first;
	fieldlines.push_back("Content-Length: " + std::to_string(body.size()));
	return HTTPMessage("HTTP/1.1 200 OK", fieldlines, body);
}

namespace cgi {
	auto	run(const Server& self, [[maybe_unused]] HTTPMessage http, const std::filesystem::path& bin, const std::string& query, struct in_addr peer_addr) -> HTTPMessage {
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

		return (pid ? parentProcedure(self, http, in,  out, pid) : childProcedure(self, http, bin, query, peer_addr, in, out));
	}
}
