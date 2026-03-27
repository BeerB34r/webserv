/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   webserv.cpp                                             :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/24 16:22:18 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/24 16:28:23 by mde-beer            ########   odam.nl   */
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

#include <cerrno>
#include <cstring>
#include <webserv.hpp>

#include <debug.hpp>
#include <Server.hpp>
#include <HTTPparsing.hpp>
#include <cgi.hpp>

#include <sys/epoll.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <dirent.h>
#include <sys/wait.h> // waitpid

#include <csignal>		// signal()
#include <filesystem>	// std::filesystem
#include <fstream>

#define PORT 8080
#define LISTEN_BACKLOG 50

static auto	autoIndex(const std::string& request, const std::filesystem::path& dirPath) -> HTTPMessage {
	std::stringstream	ss;
	ss << 
		"<!DOCTYPE html>\n<html>\n"
		"\t<title>Directory Listing</title>\n"
		"\t<body>\n"
		"\t\t<h1>Directory Listing</h1>\n"
		"\t\t<ul>\n";
	for (const std::filesystem::directory_entry &dirent : std::filesystem::directory_iterator{dirPath}) {
		std::string	name = dirent.path().filename();
		if (dirent.is_directory()) name.append("/");
		ss << "\t\t\t<li><a href=\"" + (request.ends_with('/') ? request : request + '/') + name + "\">" << name << "</a></li>\n";
	}
	ss <<
		"\t\t</ul>\n"
		"\t<body>\n"
		"</html>\n";
	return defaultpage::create200(ss.str());
}

static inline auto	isCGI(const std::filesystem::path& target, const Server& self) -> bool {
	for (const std::string& s : self.cgiExts) if (target.extension().string() == s) return true;
	for (const std::string& s : self.cgiDirs) {
		if (target.string().contains(s)) return true;
	}
	return false;
}

static inline auto	checkFile(const std::filesystem::path& file, int flag) -> int {
	if (!std::filesystem::exists(file)) return 404; // not found
	else if (access(file.c_str(), flag)) return 403; // not allowed
	return 0;
}

static auto	fulfillGetRequest(const Server& self, const HTTPMessage& http, const std::string& request, const std::filesystem::path& target, const std::string& query, struct in_addr peer_addr) -> std::variant<std::pair<int,pid_t>,HTTPMessage> {
	struct stat statbuf;

	if (int status = checkFile(target, R_OK)) {
		return self.statusPages.at(status);
	} else if (stat(target.c_str(), &statbuf)) {
		return self.statusPages.at(403);
	} else switch (statbuf.st_mode & S_IFMT) {
		case (S_IFDIR): {
			if (self.values.contains("index")
				&& !self.values.at("index").empty()
				&& (request == "/" || !request.ends_with('/'))
				) {
				INFO("index: " + self.values.at("index") + ", appended path: " + target.string() + "/" + self.values.at("index"));
				return fulfillRequestTarget(self, http, request, target.string() + "/" + self.values.at("index"), query, peer_addr);
			}
			else if (self.values.contains("autoindex")) return autoIndex(request, target);
			else return self.statusPages.at(403);
		};
		case (S_IFREG): {
			std::ifstream	file(target);
			if (!file.is_open()) {
				WARN("could not open requested file");
				return self.statusPages.at(500);
			}
			std::stringstream	ss;
			ss << file.rdbuf();
			return defaultpage::create200(ss.str());
		}
		default : {
			return self.statusPages.at(403);
		}
	}
}

static auto	fulfillPostRequest(const Server& self, const HTTPMessage& http, [[maybe_unused]] const std::string& request, const std::filesystem::path& target, [[maybe_unused]] const std::string& query) -> HTTPMessage {
	bool	isInDataDir = false;
	for (const std::string& s : self.dataDirs) if (target.parent_path().lexically_relative(s).lexically_normal().string() == ".") isInDataDir = true;
	if (!isInDataDir) return self.statusPages.at(403);

	// overwrite? // no
	if (std::filesystem::exists(target)) {
		return self.statusPages.at(403);
	} else { // regular write
		if (!std::filesystem::create_directories(target.parent_path())) return self.statusPages.at(500);
		std::ofstream	file(target);
		if (!file.is_open()) return self.statusPages.at(500);
		file << http.getBody();
		file.close();
		return defaultpage::create201(http, request);
	}
}

static auto	fulfillDeleteRequest(const Server& self, [[maybe_unused]] const HTTPMessage& http, [[maybe_unused]] const std::string& request, const std::filesystem::path& target, [[maybe_unused]] const std::string& query) -> HTTPMessage {
	bool	isInDataDir = false;
	for (const std::string& s : self.dataDirs) if (target.parent_path().lexically_relative(s).lexically_normal().string() == ".") isInDataDir = true;
	if (!isInDataDir) return self.statusPages.at(403);

	if (!std::filesystem::exists(target)) {
		return self.statusPages.at(404); // its already not there bruh
	} else {
		std::filesystem::remove_all(target);
		return defaultpage::create200(
			"<!DOCTYPE html>\n<html>\n"
			"\t<title>" + request + " deleted</title>\n"
			"\t<body>\n"
			"\t\t<h1>File deleted</h1>\n"
			"\t\t" + request + " has been deleted\n"
			"\t</body>\n"
			"</html>"
		);
	}
}

auto	fulfillRequestTarget(const Server& self, HTTPMessage http, const std::string& request, const std::filesystem::path& target, const std::string& query, struct in_addr peer_addr) -> std::variant<std::pair<int,pid_t>,HTTPMessage> {
	// make relative to root => make the normal form => check if begins with ..
	if (target.lexically_relative(self.root).lexically_normal().string().starts_with("..")) {
		return self.statusPages.at(403); // youre not allowed to do path traversal grr
	}

	if (!self.supportedMethods.contains(std::get<HTTPMessage::RequestData>(http.getData()).method)) {
		return self.statusPages.at(405);
	}

	if (isCGI(target, self)) {
		if (int status = checkFile(target, X_OK)) return self.statusPages.at(status);
		return cgi::run(self, http, target, query, peer_addr);
	}
	switch (std::get<HTTPMessage::RequestData>(http.getData()).method) {
		case (HTTPMessage::GET): return fulfillGetRequest(self, http, request, target, query, peer_addr);
		case (HTTPMessage::POST): return fulfillPostRequest(self, http, request, target, query);
		case (HTTPMessage::DELETE): return fulfillDeleteRequest(self, http, request, target, query);
		default: std::unreachable();
	}
}

auto	fulfillRequest(Server& self, struct epoll_event* ev, struct in_addr peer_addr) noexcept -> std::variant<std::pair<int,pid_t>,HTTPMessage> {
	std::optional<HTTPMessage>	http = readHTTPrequest(self.client_data[ev->data.fd]);
	if (http && !http->getFields().contains("Host")) return self.statusPages.at(400);
	std::optional<std::filesystem::path>	target = http.and_then([](const HTTPMessage& m) -> std::optional<std::string> {
			return HTTPparsing::originForm(std::get<HTTPMessage::RequestData>(m.getData()).requestTarget).transform([](auto p) -> std::string { return p.second;});
			}).transform([self](std::string s) -> std::string {
				// parsing prevents path traversal vulnerabilities (somehow)
				if (self.root.ends_with('/')) {
					return self.root + HTTPparsing::absolutePath(s)->second.substr(1); // cannot fail
				} else {
					return self.root + HTTPparsing::absolutePath(s)->second; // cannot fail
				}
			}); // she lambda on my calc til i ulus
	std::optional<std::string>	query = http.and_then([](const HTTPMessage& m) -> std::optional<std::string> {
			return (HTTPparsing::absolutePath > Parse::parseChar('?') > HTTPparsing::query)(std::get<HTTPMessage::RequestData>(m.getData()).requestTarget).transform([](auto p) -> std::string { return p.second;});
		}); // lego parser makes this so nice and easy to work with holee
	if (http && target)
		return (fulfillRequestTarget(self, *http, std::get<HTTPMessage::RequestData>(http->getData()).requestTarget, *target, query ? *query : "", peer_addr));
	else return (self.statusPages.at(400));
}

auto	defaultWriteEventHandler(Server& self, int pollfd, struct epoll_event* ev, [[maybe_unused]] struct in_addr peer_addr) -> bool {
	bool	should_close = false;
	if (self.hasCallback.contains(ev->data.fd)) {
		pid_t proc = self.hasCallback[ev->data.fd];
		self.callbacks[proc].first = true;
		return false;
	}
	HTTPMessage	message = self.responses.at(ev->data.fd);
	int	status = std::get<HTTPMessage::ResponseData>(message.getData()).statusCode;
	if (status > 399 || 200 > status) should_close = true;
	std::stringstream	ss;
	ss << message;
	std::string			response = ss.str();
	send(ev->data.fd, response.c_str(), response.length(), 0);
	self.client_data[ev->data.fd] = "";
	self.responses.erase(ev->data.fd);
	if (should_close) {
		close(ev->data.fd);
		self.client_data.erase(ev->data.fd);
	} else {
		ev->events = EPOLLIN | EPOLLET;
		epoll_ctl(pollfd, EPOLL_CTL_MOD, ev->data.fd, ev);
	}
	INFO("request from port " + std::to_string(self.port) + " fulfilled");
	return (should_close);
}

auto	defaultReadEventHandler(Server& self, int pollfd, struct epoll_event* ev, struct in_addr peer_addr) -> bool {
	char	buf[BUFFER_SIZE];
	int rv;
	size_t	readAmount = self.maxRequestSize < BUFFER_SIZE ? BUFFER_SIZE : self.maxRequestSize;
	size_t	totalRead = 0;
	while ((rv = recv(ev->data.fd, buf, readAmount, MSG_DONTWAIT)) > 0) {
		self.client_data[ev->data.fd].append(buf, rv);
		totalRead += rv;
		if (totalRead >= self.maxRequestSize) break ;
	}
	if (rv < 0) { // assume error is EAGAIN/EWOULDBLOCK, not allowed to check cuz fuck you
		INFO("end of current message from port " + std::to_string(self.port));
	}
	if (totalRead >= self.maxRequestSize) self.responses.insert_or_assign(ev->data.fd, self.statusPages.at(413));
	else {
		// varaint containing either:
		// A => HTTP response
		// B => callback that returns a response, and the read-end of a pipe containing a CGI process (for epoll)
		std::variant<std::pair<int,pid_t>,HTTPMessage>	requestResult = fulfillRequest(self, ev, peer_addr);
		if (std::holds_alternative<HTTPMessage>(requestResult)) self.responses.insert_or_assign(ev->data.fd, std::get<HTTPMessage>(requestResult));
		else {
			auto [ output , proc ] = std::get<std::pair<int,pid_t>>(requestResult);
			self.hasCallback[ev->data.fd] = proc;
			self.callbacks[proc] = std::make_pair(false, output);
			// prepare callback
		}
	}
	if (rv == 0) { // end of communication
		INFO("connection closed on port " + std::to_string(self.port));
		close(ev->data.fd);
		self.client_data.erase(ev->data.fd);
		self.responses.erase(ev->data.fd);
		return (true);
	}
	ev->events = EPOLLOUT | EPOLLET;
	epoll_ctl(pollfd, EPOLL_CTL_MOD, ev->data.fd, ev);
	return (false);
}

// returns negative value on error
auto	createSocket(short port, long address = INADDR_ANY) -> int {
	int	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		ERROR("failed to create network socket");
		return (-1);
	}

	struct sockaddr_in	addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(address);
	int	reuse = 1;
	// allow reuse of address and port
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		ERROR("failed while setting network socket option \"reuse address\"\n");
		goto error;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		ERROR("failed while setting network socket option \"reuse port\"");
		goto error;
	}
	// do not let the port remain open after the program closes
	linger	lin;
	lin.l_linger = 0;
	lin.l_onoff = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char *>(&lin), sizeof(lin))) {
		ERROR("failed while setting network socket option \"linger\"");
		goto error;
	}

	if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr))) {
		ERROR("failed to bind network socket at "
				+ std::to_string(address >> (8 * 3)) + "."
				+ std::to_string(address >> (8 * 2) & 0xFF) + "."
				+ std::to_string(address >> (8 * 1) & 0xFF) + "."
				+ std::to_string(address & 0xFF) + ":"
				+ std::to_string(port)
		);
		goto error;
	}
	if (listen(sock, LISTEN_BACKLOG) == -1) {
		ERROR("failed to listen on network socket");
		goto error;
	}
	return (sock);
error:
	close(sock);
	return (-1);
}

// returns negative on error
auto	getIncomingConnection(int socket, struct in_addr& peer_addr) noexcept -> int {
	struct sockaddr_in	peer;
	socklen_t	peerAddrSize = sizeof(peer);
	int	out = accept(socket, reinterpret_cast<struct sockaddr *>(&peer), &peerAddrSize);
	if (out < 0) WARN("failed to accept incoming traffic");
	else peer_addr = peer.sin_addr;
	return out;
}

bool	stop;

auto	intHandler([[maybe_unused]] int signum) noexcept -> void {
	stop = true;
	INFO("SIGINT recieved, exiting gracefully");
}

static auto	closeMap(std::map<int,Server>&	fds) noexcept -> void {
	for (auto fd : fds) close(fd.first);
}

auto	webserv(const std::vector<Server>& servers) noexcept -> int {
	using namespace std::literals;
	using fd = int;
	int	server_rv = 0;

	// set up network sockets
	std::map<fd,Server>	listeners;
	INFO("server count: " + std::to_string(servers.size()));
	for (const Server& s : servers) {
		const fd	sock = createSocket(s.port, s.address);
		if (sock < 0) continue ;
		listeners[sock] = s;
	}
	if (listeners.empty()) {
		FATAL("Failed to instantiate any listening sockets");
		return (1); // cuz you only need the light when its burnin low
	}

	INFO("listener count: " + std::to_string(listeners.size()));

	// set up epoll
	struct epoll_event	events[MAX_EVENTS];
	const fd	pollfd = epoll_create1(0); // passenger riff
	if (pollfd < 0) {
		FATAL("failed to create epoll instance");
		closeMap(listeners);
		return (1);
	}

	// add the listening sockets into epoll
	struct epoll_event	ev;
	for (std::pair<const fd,Server> sock : listeners) {
		ev.events = EPOLLIN;
		ev.data.fd = sock.first;
		if (epoll_ctl(pollfd, EPOLL_CTL_ADD, sock.first, &ev) < 0) {
			FATAL("failed to add network socket into epoll instance");
			close(pollfd); // only miss the sun when it starts to snow
			closeMap(listeners);
			return (1);
		}
	}

	__sighandler_t	originalIntHandler = signal(SIGINT, intHandler);
	int message_count = 0;
	std::map<fd,Server>		sockToServer;
	std::map<fd,struct in_addr>	sockToAddr;
	while (!stop || server_rv) {
		int	nfds = epoll_wait(pollfd, events, MAX_EVENTS, 0);
		for (int n = 0; n < nfds; n++) {
			struct epoll_event	*current = &events[n];
			int	socket = current->data.fd;

			if (listeners.contains(socket)) { // new connection
				struct in_addr peer_addr;
				fd	connection_socket = getIncomingConnection(socket, peer_addr);
				if (connection_socket < 0) continue ;
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = connection_socket;
				if (epoll_ctl(pollfd, EPOLL_CTL_ADD, connection_socket, &ev) < 0) {
					FATAL("failed to add incoming connection to epoll instance");
					close(connection_socket); // only know you love her when you let her go
					server_rv = 1;
					break ;
				}
				sockToServer[connection_socket] = listeners[socket];
				sockToAddr[connection_socket] = peer_addr;
				INFO("opened connection on port " + std::to_string(listeners[socket].port));
			} else if (current->events & EPOLLERR) { // error on socket
					close(socket);
					Server& server = sockToServer.at(socket);
					if (server.hasCallback.contains(socket)) {
						kill(server.hasCallback.at(socket), SIGKILL);
						server.callbacks.erase(server.hasCallback.at(socket));
						server.hasCallback.erase(socket);
					}
					sockToServer.erase(socket);
					sockToAddr.erase(socket);
			} else { // client read/write event
				Server&	serverConfig = sockToServer[socket];
				struct in_addr	peer_addr = sockToAddr[socket];
				if (current->events & EPOLLIN
					? serverConfig.readEventHandler(serverConfig, pollfd, current, peer_addr)
					: serverConfig.writeEventHandler(serverConfig, pollfd, current, peer_addr)
					) { // can we pretend that airplanes in the night sky are like shooting stars
					sockToServer.erase(socket);
					sockToAddr.erase(socket);
				}
				message_count += !(current->events & EPOLLIN);
			}
		}
		// check if child is doing shit
		for (std::pair<const fd, Server>& a : sockToServer) {
			Server &s = a.second;
			if (s.hasCallback.empty()) continue ;
			for (std::pair<const int, pid_t> p : s.hasCallback) {
				int	sock = p.first;
				pid_t proc = p.second;
				int status;
				if (int rv = waitpid(proc, &status, WNOHANG) != p.second) {
					if (rv == 0) continue ; // WNOHANG, child is still running
					if (rv < 0) {
						s.responses.insert_or_assign(sock, s.statusPages.at(500));
						s.callbacks.erase(proc);
						s.hasCallback.erase(sock);
					}
				};
				auto& [ ready, output ]  = s.callbacks[proc];
				if (!ready) {
					s.responses.insert_or_assign(sock, cgi::callback(output, s, proc));
					s.callbacks.erase(proc);
					s.hasCallback.erase(sock);
				} else {
					HTTPMessage http = cgi::callback(output, s, proc);
					std::stringstream ss;
					ss << http;
					std::string response = ss.str();
					send(sock, response.c_str(), response.size(), 0);
					sockToServer.erase(sock);
					sockToAddr.erase(sock);
				}
			}
		}
	}
	signal(SIGINT, originalIntHandler);
	// KILL ALL KIDS
	for (std::pair<const fd, Server>& a : sockToServer) {
		Server& s = a.second;
		if (s.hasCallback.empty()) continue ;
		for (std::pair<const int, pid_t> p : s.hasCallback) kill(p.second, SIGKILL);
	}
	closeMap(listeners);
	close(pollfd);
	INFO(+ std::to_string(message_count) + " messages processed");
	return (server_rv);
}
