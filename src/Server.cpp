/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Server.cpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/10 19:57:48 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/12 21:11:31 by mde-beer            ########   odam.nl   */
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

#include <Server.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <charconv>
#include <cstring>
#include <sstream>
#include <fstream>
#include <csignal>
#include <functional>
#include <map>
#include <filesystem>

#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <debug.hpp>
#include <HTTPMessage.hpp>
#include <HTTPparsing.hpp>
#include <defaultpage.hpp>

// allow for std::string literals etc
using namespace std::literals;

static inline auto	to_int(const std::string& s) noexcept -> std::optional<int> {
	int	rv{};
	if (std::from_chars(s.data(), s.data() + s.size(), rv).ec == std::errc{}) {
		return rv;
	}
	else return std::nullopt;
}

static auto	autoIndex(Server& self, const std::string& request, const std::filesystem::path& dirPath) -> HTTPMessage {
	DIR* dir = opendir(dirPath.c_str());
	if (!dir) {
		WARN("could not open requested directory");
		return self.statusPages.at(500);
	}
	struct dirent	*dirent;
	std::stringstream	ss;
	ss << "<!DOCTYPE html>\n<html>\n";
	for (ss << "\t<title>Directory Listing</title>\n\t<body>\n\t\t<h1>Directory Listing</h1>\n\t\t<ul>\n"; (dirent = readdir(dir));) {
		std::string	name = dirent->d_name;
		switch (dirent->d_type) {
			case (DT_DIR): {
				name += '/';
				break ;
			} case (DT_REG): {
				break ;
			} default: {
				break ;
			}
		}
		ss << "\t\t\t<li><a href=\"" + (request.ends_with('/') ? request : request + '/') + name + "\">" << name << "</a></li>\n";
	}
	ss << "\t\t</ul>\n\t<body>\n</html>\n";
	closedir(dir);
	return defaultpage::create200(ss.str());
}

#include <cgi.hpp>

static inline auto	isCGI(const std::filesystem::path& target, const Server& self) -> bool {
	INFO("target.extension = " + target.extension().string());
	for (const std::string& s : self.cgiExts) if (target.extension().string() == s) return true;
	for (const std::string& s : self.cgiDirs) {
		INFO(+ target.string());
		if (target.string().contains(s)) return true;
	}
	if (target.parent_path().string().contains("/cgi-bin/")) return true;
	return false;
}

static inline auto	checkFile(const std::filesystem::path& file, int flag) -> int {
	if (!std::filesystem::exists(file)) return 404; // not found
	else if (access(file.c_str(), flag)) return 403; // not allowed
	return 0;
}

static auto	fulfillGetRequest(Server& self, HTTPMessage http, const std::string& request, const std::filesystem::path& target) -> HTTPMessage {
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
				return fulfillRequestTarget(self, http, request, target.string() + "/" + self.values.at("index"));
			}
			else if (self.values.contains("autoindex")) return autoIndex(self, request, target);
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

auto	fulfillRequestTarget(Server& self, HTTPMessage http, const std::string& request, const std::filesystem::path& target) -> HTTPMessage {
	// make relative to root => make the normal form => check if begins with ..
	if (target.lexically_relative(self.root).lexically_normal().string().starts_with("..")) {
		return self.statusPages.at(403); // youre not allowed to do path traversal grr
	}

	if (!self.supportedMethods.contains(std::get<HTTPMessage::RequestData>(http.getData()).method)) {
		return self.statusPages.at(405);
	}

	// file extension based CGI
	if (isCGI(target, self)) {
		if (int status = checkFile(target, X_OK)) return self.statusPages.at(status);
		return cgi::run(self, http, target);
	}
	switch (std::get<HTTPMessage::RequestData>(http.getData()).method) {
		case (HTTPMessage::GET): return fulfillGetRequest(self, http, request, target);
		case (HTTPMessage::POST): return self.statusPages.at(500);
		case (HTTPMessage::DELETE): return self.statusPages.at(500);
		default: std::unreachable();
	}
}

#define BUFFER_SIZE 1024
auto	defaultWriteEventHandler(Server& self, [[maybe_unused]] int pollfd, struct epoll_event* ev) -> bool {
	bool	should_close = false;
	std::stringstream	ss;
	std::optional<HTTPMessage>	http = readHTTPrequest(self.client_data[ev->data.fd]);
	std::optional<std::filesystem::path>	target = http.and_then([](const HTTPMessage& m) -> std::optional<std::string> {
			return HTTPparsing::originForm(std::get<HTTPMessage::RequestData>(m.getData()).requestTarget).transform([](auto p) -> std::string { return p.second;});;
			}).transform([self](std::string s) -> std::string {
				// parsing prevents path traversal vulnerabilities (somehow)
				if (self.root.ends_with('/')) {
					return self.root + HTTPparsing::absolutePath(s)->second.substr(1); // cannot fail
				} else {
					return self.root + HTTPparsing::absolutePath(s)->second; // cannot fail
				}
			}); // she lambda on my calc til i ulus
	if (http // is the http valid?
		&& target // is there a valid target?
		) ss << fulfillRequestTarget(self, *http, std::get<HTTPMessage::RequestData>(http->getData()).requestTarget, *target);
	else ss << self.statusPages.at(400);
	send(ev->data.fd, ss.str().c_str(), ss.str().length(), 0);
	self.client_data[ev->data.fd] = "";
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

auto	defaultReadEventHandler(Server& self, int pollfd, struct epoll_event* ev) -> bool {
	char	buf[BUFFER_SIZE];
	int rv;
	while ((rv = recv(ev->data.fd, buf, BUFFER_SIZE, MSG_DONTWAIT)) > 0)
		self.client_data[ev->data.fd].append(buf, rv);
	if (rv < 0) { // assume error is EAGAIN, not allowed to check cuz fuck you
		INFO("end of current message from port " + std::to_string(self.port));
	};
	if (rv == 0) {
		INFO("connection closed on port " + std::to_string(self.port));
		close(ev->data.fd);
		self.client_data.erase(ev->data.fd);
		return (true);
	}
	// should probably loop so we get the entire message because ET
	ev->events = EPOLLOUT | EPOLLET;
	epoll_ctl(pollfd, EPOLL_CTL_MOD, ev->data.fd, ev);
	return (false);
}

static auto	ipv4ToLong(const std::string& s) noexcept -> long {
	std::string	addr = (s == "localhost") ? "127.0.0.1" : s;
	long	rv;
	long	firstOctet = *to_int(addr);
	long	secondOctet = *to_int(addr.substr(addr.find('.') + 1));
	long	thirdOctet = *to_int(addr.substr(addr.find('.', addr.find('.') + 1) + 1));
	long	fourthOctet = *to_int(addr.substr(addr.find('.', addr.find('.', addr.find('.') + 1) + 1) + 1));

	rv = (firstOctet << (8 * 3)) + (secondOctet << (8 * 2)) + (thirdOctet << (8 * 1)) + (fourthOctet << (8 * 0));
	return rv;
}

static auto	readFile(const std::string& path) -> Maybe<std::string> {
	std::ifstream	file(path);
	if (!file.is_open()) return std::nullopt;
	std::stringstream	ss;
	ss << file.rdbuf();
	return ss.str();
}

static inline auto	splitOnChar(std::string s, char c) -> std::vector<std::string> {
	std::vector<std::string>	rv;
	s.push_back(c);
	do {
		rv.push_back(s.substr(0, s.find(c)));
		s = s.substr(s.find(c) + 1);
	} while (s.size());
	return rv;
}

static auto	singleServerFromConfig(const Config& c) -> Maybe<Server> {
	Server s;
	s.blocks = c.blocks;
	s.ident = c.ident;
	s.values = c.values;
	if (c.values.at("listen").contains(':')) {
		std::string	prefix = s.values.at("listen").substr(0, s.values.at("listen").find(':'));
		std::string	suffix = s.values.at("listen").substr(s.values.at("listen").find(':') + 1);
		if (prefix == "localhost") prefix = "127.0.0.1";
		s.address = ipv4ToLong(prefix);
		s.port = *to_int(suffix);
	} else { // any address
		s.port = *to_int(c.values.at("listen"));
	}
	if (c.contains(Config::ERROR)) {
		for (Config current : c.getBlocks(Config::ERROR)) for (std::pair<std::string,std::string> p : current.values) {
			if (!to_int(p.first)) continue ;
			int	code = *to_int(p.first);
			Maybe<std::string>	page = readFile(p.second);
			if (!page) {
				WARN("could not read error page " + p.second);
				return std::nullopt;
			}
			if (s.statusPages.contains(code)) {
				s.statusPages.insert_or_assign(code, HTTPMessage(
							s.statusPages.at(code).getStartline(),
							{"content-length:" + std::to_string(page->size())},
							*page
				));
			} else {
				WARN("error code " + std::to_string(code) + " is not supported" );
			}
		}
	}
	if (c.values.contains("cgi")) for (const std::string& val : splitOnChar(c.values.at("cgi"), ',')) s.cgiExts.insert(val);
	if (c.values.contains("cgidir")) for (const std::string& val : splitOnChar(c.values.at("cgidir"), ',')) s.cgiDirs.insert(val);
	if (c.values.contains("allowedmethods")) {
		for (const std::string& val : splitOnChar(c.values.at("allowedmethods"), ',')) s.supportedMethods.insert(toHTTPMethod(val));
		for (HTTPMessage::HTTPMethod m : s.supportedMethods) {
			if (!HTTPMessage::supportedRequestMethods.contains(m)) {
				WARN("unsupported method in config file");
				return std::nullopt;
			}
		}
	} else s.supportedMethods = HTTPMessage::supportedRequestMethods;
	s.root = c.values.at("root").substr(0, c.values.at("root").find(','));
	s.writeEventHandler = defaultWriteEventHandler;
	s.readEventHandler = defaultReadEventHandler;
	return s;
}

auto	fromConfig(const Config& c) -> std::vector<Server> {
	std::vector<Server>	rv;

	if (!c.contains(Config::SERVER)) {
		Maybe<Server> s = singleServerFromConfig(c);
		if (s)
			rv.push_back(*s);
	} else for (Config& current : c.getBlocks(Config::SERVER)) {
		Maybe<Server> s = singleServerFromConfig(current);
		if (s)
			rv.push_back(*s);
	}
	return rv;
}

#define PORT 8080
#define LISTEN_BACKLOG 50

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
auto	getIncomingConnection(int socket) -> int {
	struct sockaddr_in	peer;
	socklen_t	peerAddrSize = sizeof(peer);
	int	out = accept(socket, reinterpret_cast<struct sockaddr *>(&peer), &peerAddrSize);
	if (out < 0) WARN("failed to accept incoming traffic");
	return out;
}

typedef std::function<HTTPMessage(Maybe<HTTPMessage>)> RequestHandler;

bool	stop;

auto	intHandler([[maybe_unused]] int signum) -> void {
	stop = true;
	INFO("SIGINT recieved, exiting gracefully");
}

static auto	closeMap(std::map<int,Server>&	fds) -> void {
	for (auto fd : fds) close(fd.first);
}

#define MAX_EVENTS 10
auto	mvpServer(const std::vector<Server>& servers) -> int {
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
	while (!stop || server_rv) {
		int	nfds = epoll_wait(pollfd, events, MAX_EVENTS, 0);
		for (int n = 0; n < nfds; n++) {
			struct epoll_event	*current = &events[n];
			int	socket = current->data.fd;

			if (listeners.contains(socket)) { // new connection
				fd	connection_socket = getIncomingConnection(socket);
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
				INFO("opened connection on port " + std::to_string(listeners[socket].port));
			} else  { // new client event
				Server&	serverConfig = sockToServer[socket];
				if (current->events & EPOLLIN
					? serverConfig.readEventHandler(serverConfig, pollfd, current)
					: serverConfig.writeEventHandler(serverConfig, pollfd, current)
					) // can we pretend that airplanes in the night sky are like shooting stars
					sockToServer.erase(socket);
				message_count += !(current->events & EPOLLIN);
			}
		}
	}
	signal(SIGINT, originalIntHandler);
	closeMap(listeners);
	close(pollfd);
	INFO(+ std::to_string(message_count) + " messages processed");
	return (server_rv);
}
