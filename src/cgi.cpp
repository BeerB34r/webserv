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

static inline auto	childProcedure [[noreturn]] (const Server& self [[maybe_unused]], const std::filesystem::path& bin, int in[2], int out[2]) -> HTTPMessage {
	close(in[1]);
	close(out[0]);
	dup2(in[0], STDIN_FILENO);
	dup2(out[1], STDOUT_FILENO);
	execve(extensionArgs(bin).front().c_str(), createExecveArg(extensionArgs(bin)).get(), createExecveArg({}).get());
	FATAL("execve failed"); // execve should never fail here
	exit(1);
}

// TODO: add this to epoll so it isnt blocking
static inline auto	parentProcedure (const Server& self, [[maybe_unused]] const std::string& bin, int in[2], int out[2], pid_t child) -> HTTPMessage {
	close(in[0]);
	close(out[1]);
	// TODO: write to in[1]
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
	auto	run(const Server& self, [[maybe_unused]] HTTPMessage http, const std::filesystem::path& bin, const std::string& query) -> HTTPMessage {
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

		return (pid ? parentProcedure(self, bin, in,  out, pid) : childProcedure(self, bin, in, out));
	}
}
