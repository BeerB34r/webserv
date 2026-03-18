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

#include <cstring>
#include <defaultpage.hpp>
#include <cgi.hpp>
#include <debug.hpp>
#include <memory>
#include <unistd.h>
#include <HTTPparsing.hpp>
#include <sys/wait.h>
#include <fcntl.h>
#include <filesystem>

#define BUFFER_SIZE 1024

using namespace std::literals;

static auto	createExecveArg(std::vector<std::string> v) -> std::unique_ptr<char*[]>{;
	std::unique_ptr<char*[]>	rv(new char*[v.size() + 1]);

	int i = 0;
	for (std::string& s : v) {
		rv[i++] = s.data();
	}
	rv[i] = NULL;
	return rv;
}

static inline auto	extensionArgs(const std::filesystem::path& bin) -> std::vector<std::string> {
	std::vector<std::string>	rv;

	if (!bin.has_extension())
		return {bin};
	if (bin.extension().string() == ".hs")
		return {"/usr/bin/env", "-S", "/home/mde-beer/sgoinfre/.ghcup/bin/ghc", "--run", bin}; // normally this would just say /usr/bin/env -S ghc, but since ghc isnt on the system path (i think) it doesnt work
	if (bin.extension().string() == ".py")
		return {"/usr/bin/env", "-S", "python3", bin};
	return {bin};
}

static inline auto	childProcedure [[noreturn]] (Server& self [[maybe_unused]], const std::filesystem::path& bin, int pipe[2]) -> HTTPMessage {
	close(pipe[0]);
	dup2(pipe[1], STDOUT_FILENO);
	execve(extensionArgs(bin).front().c_str(), createExecveArg(extensionArgs(bin)).get(), createExecveArg({}).get());
	FATAL("execve failed"); // execve should never fail here
	exit(1);
}

// TODO: add this to epoll so it isnt blocking
static inline auto	parentProcedure (Server& self, [[maybe_unused]] const std::string& bin, int pipe[2], pid_t child) -> HTTPMessage {
	close(pipe[1]);
	if (waitpid(child, NULL, 0) < 0) return self.statusPages.at(500);
	std::string	rv;
	char buf[BUFFER_SIZE];
	int	bytes;
	do {
		bytes = read(pipe[0], buf, BUFFER_SIZE);
		rv.append(buf, bytes);
	} while (bytes != 0);
	close(pipe[0]);
	Maybe<Pair<std::string,std::vector<std::string>>>	parseRes = (HTTPparsing::fieldLines < HTTPparsing::crlf)(rv); // holy type
	if (!parseRes) return self.statusPages.at(500);
	std::vector<std::string>	fieldlines = parseRes->second;
	std::string	body = parseRes->first;
	fieldlines.push_back("Content-Length: " + std::to_string(body.size()));
	return HTTPMessage("HTTP/1.1 200 OK", fieldlines, body);
}

namespace cgi {
	auto	run(Server& self, [[maybe_unused]] HTTPMessage http, const std::filesystem::path& bin) -> HTTPMessage {
		using fd = int;

		fd	fds[2];
		if (pipe2(fds, O_NONBLOCK)) return self.statusPages.at(500); // O_NONBLOCK for whenever i put this in epoll

		const pid_t	pid = fork();
		if (pid < 0) return self.statusPages.at(500);

		return (pid ? parentProcedure(self, bin, fds, pid) : childProcedure(self, bin, fds));
	}
}
