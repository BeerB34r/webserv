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
#include <fcntl.h>

#include <csignal>		// signal()
#include <filesystem>	// std::filesystem
#include <fstream>

#define PORT 8080
#define LISTEN_BACKLOG 50

#define _evStr(s, e, m) do {\
	if (e & m) \
		s.empty() ? s = #m : s += "|"s + #m;\
}	while(0)
static inline auto	epollEventToString(int e) {
	std::string	rv = "";
	_evStr(rv, e, EPOLLIN);
	_evStr(rv, e, EPOLLPRI);
	_evStr(rv, e, EPOLLOUT);
	_evStr(rv, e, EPOLLRDNORM);
	_evStr(rv, e, EPOLLRDBAND);
	_evStr(rv, e, EPOLLWRNORM);
	_evStr(rv, e, EPOLLWRBAND);
	_evStr(rv, e, EPOLLMSG);
	_evStr(rv, e, EPOLLERR);
	_evStr(rv, e, EPOLLHUP);
	_evStr(rv, e, EPOLLRDHUP);
	_evStr(rv, e, EPOLLEXCLUSIVE);
	_evStr(rv, e, EPOLLWAKEUP);
	_evStr(rv, e, EPOLLONESHOT);
	_evStr(rv, e, EPOLLET);
	return rv;
}

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

static auto	fulfillGetRequest(const Server& self, const HTTPMessage& http, const std::string& request, const std::filesystem::path& target, const std::string& query, struct in_addr peer_addr) -> std::variant<std::pair<int,Process>,HTTPMessage> {
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
		std::error_code	ec;
		std::filesystem::create_directories(target.parent_path(), ec);
		if (ec) return self.statusPages.at(500);
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

auto	fulfillRequestTarget(const Server& self, HTTPMessage http, const std::string& request, const std::filesystem::path& target, const std::string& query, struct in_addr peer_addr) -> std::variant<std::pair<int,Process>,HTTPMessage> {
	// make relative to root => make the normal form => check if begins with ..
	std::string	serverRoot = self.root;
	for (const std::pair<const std::string, std::pair<std::string,std::set<HTTPMessage::HTTPMethod>>>& p : self.routes) {
		if (request.starts_with(p.first))
			serverRoot = p.second.first;
		else continue ;
		if (target.lexically_relative(serverRoot).lexically_normal().string().starts_with("..")) {
			return self.statusPages.at(403); // youre not allowed to do path traversal grr
		}
		if (!p.second.second.contains(std::get<HTTPMessage::RequestData>(http.getData()).method)) {
			return self.statusPages.at(405);
		}
	}
	if (serverRoot == self.root) {
		if (target.lexically_relative(serverRoot).lexically_normal().string().starts_with("..")) {
			return self.statusPages.at(403); // youre not allowed to do path traversal grr
		}
		if (!self.supportedMethods.contains(std::get<HTTPMessage::RequestData>(http.getData()).method)) {
			return self.statusPages.at(405);
		}
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

auto	fulfillRequest(Server& self, struct epoll_event* ev, struct in_addr peer_addr) noexcept -> std::variant<std::pair<int,Process>,HTTPMessage> {
	std::optional<HTTPMessage>	http = readHTTPrequest(self.client_data[ev->data.fd]);
	if (http && !http->getFields().contains("Host")) return self.statusPages.at(400);
	std::optional<std::filesystem::path>	target = http.and_then([](const HTTPMessage& m) -> std::optional<std::string> {
			return HTTPparsing::originForm(std::get<HTTPMessage::RequestData>(m.getData()).requestTarget).transform([](auto p) -> std::string { return p.second;});
			}).transform([self](std::string s) -> std::string {
				std::string	absolutePath = HTTPparsing::absolutePath(s)->second; // cannot fail
				for (const std::pair<const std::string, std::pair<std::string,std::set<HTTPMessage::HTTPMethod>>>& p : self.routes) {
					if (absolutePath.starts_with(p.first)) {
						return p.second.first + absolutePath.substr(p.first.size());
					}
				}
				if (self.root.ends_with('/')) {
					return self.root + absolutePath.substr(1);
				} else {
					return self.root + absolutePath;
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
		Process& p = self.hasCallback[ev->data.fd];
		p.clientReady = true;
		return false;
	}
	HTTPMessage	message = self.responses.contains(ev->data.fd) ? self.responses.at(ev->data.fd) : self.statusPages.at(500);
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
	size_t	readAmount = BUFFER_SIZE;
	size_t	totalRead = 0;
	std::optional<HTTPMessage>	http = std::nullopt;
	while ((rv = recv(ev->data.fd, buf, readAmount, MSG_DONTWAIT)) > 0) {
		self.client_data[ev->data.fd].append(buf, rv);
		totalRead += rv;
		if (totalRead > self.maxRequestSize) break ;
		http = readHTTPrequest(self.client_data[ev->data.fd]); // inefficient, but like, lets benchmark it first lmao // its fine
		if (http && http->getBody().size() > self.maxBodySize) break ;
	}
	if (rv < 0) { // assume error is EAGAIN/EWOULDBLOCK, not allowed to check cuz fuck you // should show up in EPOLL_ERR if its bad anyway
		INFO("end of current message from port " + std::to_string(self.port));
	}
	if (totalRead > self.maxRequestSize || (http && http->getBody().size() > self.maxBodySize)) self.responses.insert_or_assign(ev->data.fd, self.statusPages.at(413));
	else {
		// varaint containing either:
		// A => HTTP response
		// B => callback that returns a response, and the read-end of a pipe containing a CGI process (for epoll)
		std::variant<std::pair<int,Process>,HTTPMessage>	requestResult = fulfillRequest(self, ev, peer_addr);
		if (std::holds_alternative<HTTPMessage>(requestResult)) self.responses.insert_or_assign(ev->data.fd, std::get<HTTPMessage>(requestResult));
		else {
			auto [ output , proc ] = std::get<std::pair<int,Process>>(requestResult);
			proc.client = ev->data.fd;
			self.hasCallback[ev->data.fd] = proc;
			struct epoll_event	proc_ev;
			proc_ev.data.fd = proc.in;
			proc_ev.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
			if (epoll_ctl(pollfd, EPOLL_CTL_ADD, proc_ev.data.fd, &proc_ev)) {
				self.responses.insert_or_assign(ev->data.fd, self.statusPages.at(500));
				kill(proc.pid, SIGKILL);
				close(proc.in);
				close(proc.out);
			}
			else {
				proc_ev.data.fd = proc.out;
				proc_ev.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(pollfd, EPOLL_CTL_ADD, proc_ev.data.fd, &proc_ev)) {
					self.responses.insert_or_assign(ev->data.fd, self.statusPages.at(500));
					kill(proc.pid, SIGKILL);
					close(proc.in);
					close(proc.out);
				}
			}

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
	fcntl(sock, F_SETFD, fcntl(sock, F_GETFD, 0) | FD_CLOEXEC);

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
	if (out < 0) {
		WARN("failed to accept incoming traffic");
		fcntl(out, F_SETFD, fcntl(out, F_GETFD, 0) | FD_CLOEXEC);
	}
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

	__sighandler_t	originalIntHandler = std::signal(SIGINT, intHandler);
	__sighandler_t	originalTermHandler = std::signal(SIGTERM, intHandler);
	int message_count = 0;
	std::map<fd,Server>		sockToServer;
	std::map<fd,struct in_addr>	sockToAddr;
	std::map<fd,Process>	procFds;
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
			} else if (procFds.contains(socket)) {
				Server&	serverConfig = sockToServer[procFds[socket].client];
				Process&	p = serverConfig.hasCallback[procFds[socket].client];
				auto	doResponse = [p](HTTPMessage http, Server& server, std::map<fd,Server>& sockToServer, std::map<fd,struct in_addr>& sockToAddr, std::map<fd,Process>& procFds) {
					if (p.clientReady) {
						std::stringstream	ss;
						ss << http;
						std::string	response = ss.str();
						send(p.client, response.c_str(), response.size(), 0);
						close(p.client);
						sockToServer[p.client].hasCallback.erase(p.client);
						sockToServer.erase(p.client);
						sockToAddr.erase(p.client);
						procFds.erase(p.out);
					} else {
						server.responses.insert_or_assign(p.client, http);
						server.hasCallback.erase(p.client);
						procFds.erase(p.out);
					}
				};
				if (current->events & EPOLLIN) {
					int rv;
					char buf[BUFFER_SIZE];
					do {
						rv = read(p.out, buf, BUFFER_SIZE);
						if (rv < 0) break ;
						p.stdoutContent.append(buf, rv);
					} while (rv > 0);
					if (rv == 0){
						INFO("cgi finished, sending to client");
						if (p.pid != 0)
							kill(p.pid, SIGKILL);
						close(p.out);
						std::optional<std::pair<std::string,std::vector<std::string>>> parsed = (HTTPparsing::fieldLines < HTTPparsing::crlf)(p.stdoutContent);
						if (!parsed) {
							doResponse(serverConfig.statusPages.at(500), serverConfig, sockToServer, sockToAddr, procFds);
							continue ;
						}
						std::vector<std::string>	fieldlines = parsed->second;
						std::string	status = "";
						bool	found_status = false, found_length = false;
						for (size_t i = 0; i < fieldlines.size(); ++i) {
							if (fieldlines[i].starts_with("Status:")) {
								status = fieldlines[i].substr(fieldlines[i].find(':') + 1);
								fieldlines.erase(fieldlines.begin() + i);
								if (found_status) {
									doResponse(serverConfig.statusPages.at(500), serverConfig, sockToServer, sockToAddr, procFds);
									continue ;
								}
								found_status = true;
							} else if (fieldlines[i].starts_with("Content-Length:")) {
								if (found_length) {
									doResponse(serverConfig.statusPages.at(500), serverConfig, sockToServer, sockToAddr, procFds);
									continue ;
								}
								found_length = true;
							}
						}
						std::string	body = parsed->first;
						if (!found_length) fieldlines.push_back("Content-Length: " + std::to_string(body.size()));
						doResponse(status.empty() ? HTTPMessage("HTTP/1.1 200 OK", fieldlines, body) : HTTPMessage("HTTP/1.1 " + status, fieldlines, body),
								serverConfig,
								sockToServer,
								sockToAddr,
								procFds);
					}
				} else if (current->events & EPOLLOUT) {
					write(p.in, p.stdinContent.c_str(), p.stdinContent.size());
					close(p.in);
					p.in = -1;
					procFds.erase(p.in);
				} else if (current->events & EPOLLHUP) {
					INFO("cgi finished, sending to client");
					close(p.out);
					std::optional<std::pair<std::string,std::vector<std::string>>> parsed = (HTTPparsing::fieldLines < HTTPparsing::crlf)(p.stdoutContent);
					if (!parsed) {
						doResponse(serverConfig.statusPages.at(500), serverConfig, sockToServer, sockToAddr, procFds);
						continue ;
					}
					std::vector<std::string>	fieldlines = parsed->second;
					std::string	status = "";
					bool	found_status = false, found_length = false;
					for (size_t i = 0; i < fieldlines.size(); ++i) {
						if (fieldlines[i].starts_with("Status:")) {
							status = fieldlines[i].substr(fieldlines[i].find(':') + 1);
							fieldlines.erase(fieldlines.begin() + i);
							if (found_status) {
								doResponse(serverConfig.statusPages.at(500), serverConfig, sockToServer, sockToAddr, procFds);
								continue ;
							}
							found_status = true;
						} else if (fieldlines[i].starts_with("Content-Length:")) {
							if (found_length) {
								doResponse(serverConfig.statusPages.at(500), serverConfig, sockToServer, sockToAddr, procFds);
								continue ;
							}
							found_length = true;
						}
					}
					std::string	body = parsed->first;
					if (!found_length) fieldlines.push_back("Content-Length: " + std::to_string(body.size()));
					doResponse(status.empty() ? HTTPMessage("HTTP/1.1 200 OK", fieldlines, body) : HTTPMessage("HTTP/1.1 " + status, fieldlines, body),
							serverConfig,
							sockToServer,
							sockToAddr,
							procFds);
				} else INFO("some non I/O event happened: " + std::to_string(current->events));
			} else if (current->events & EPOLLERR) { // error on socket
					close(socket);
					INFO("ERR on fd " + std::to_string(socket) + ", isProc: " + (procFds.contains(socket) ? "true" : "false"));
					if (sockToServer.contains(socket)) {
						Server& server = sockToServer.at(socket);
						if (server.hasCallback.contains(socket)) {
							server.hasCallback.erase(socket);
						}
						sockToServer.erase(socket);
						sockToAddr.erase(socket);
					} else {
						Server&	serverConfig = sockToServer[procFds[socket].client];
						Process&	p = serverConfig.hasCallback[procFds[socket].client];
						if (p.in == current->data.fd) {
							close(p.in);
							procFds.erase(p.in);
						} else {
							auto doResponse = [p](HTTPMessage http, Server& server, std::map<fd,Server>& sockToServer, std::map<fd,struct in_addr>& sockToAddr, std::map<fd,Process>& procFds) {
								if (p.clientReady) {
									std::stringstream	ss;
									ss << http;
									std::string	response = ss.str();
									send(p.client, response.c_str(), response.size(), 0);
									close(p.client);
									sockToServer.erase(p.client);
									sockToAddr.erase(p.client);
									procFds.erase(p.out);
								} else {
									server.responses.insert_or_assign(p.client, http);
									server.hasCallback.erase(p.client);
									procFds.erase(p.out);
								}
							};
							doResponse(serverConfig.statusPages.at(500), serverConfig, sockToServer, sockToAddr, procFds);
						}
					}
			} else if (sockToServer.contains(socket) && (current->events & EPOLLIN || current->events & EPOLLOUT)) { // client read/write event
				Server&	serverConfig = sockToServer[socket];
				struct in_addr	peer_addr = sockToAddr[socket];
				bool	inputEvent = current->events & EPOLLIN;
				if (!serverConfig.readEventHandler || !serverConfig.writeEventHandler) {
					INFO ("faulty copy on event: " + epollEventToString(current->events));
					struct stat sbuf = {};
					fstat(socket, &sbuf);
					switch (sbuf.st_mode) {
						case S_IFSOCK: INFO("socket"); break ;
						case S_IFIFO: INFO("pipe"); break ;
					}
					close(socket);
					sockToAddr.erase(socket);
					sockToServer.erase(socket);
					continue ;
				}
				if (inputEvent
					? serverConfig.readEventHandler(serverConfig, pollfd, current, peer_addr)
					: serverConfig.writeEventHandler(serverConfig, pollfd, current, peer_addr)
					) { // can we pretend that airplanes in the night sky are like shooting stars
					sockToServer.erase(socket);
					sockToAddr.erase(socket);
				}
				if (inputEvent && sockToServer.contains(socket) && sockToServer[socket].hasCallback.contains(socket)) {
					Process& p = sockToServer[socket].hasCallback[socket];
					procFds.insert_or_assign(p.in, p);
					procFds.insert_or_assign(p.out, p);
				}
				message_count += !(current->events & EPOLLIN);
			} else if (current->events & EPOLLHUP) {
				INFO("EPOLLHUP, erasing client data ");
				close(socket);
				if (sockToServer.contains(socket)) {
					Server&	server = sockToServer[socket];
					if (server.hasCallback.contains(socket)) {
						Process p = server.hasCallback[socket];
						close(p.in);
						close(p.out);
						server.hasCallback.erase(socket);
					}
				}
				sockToAddr.erase(socket);
				sockToServer.erase(socket);
			} else {
				INFO ("weird shit happened while trying to process: " + epollEventToString(current->events));
				struct stat sbuf = {};
				fstat(socket, &sbuf);
				switch (sbuf.st_mode) {
					case S_IFSOCK: INFO("socket"); break ;
					case S_IFIFO: INFO("pipe"); break ;
				}
				close(socket);
				sockToAddr.erase(socket);
				sockToServer.erase(socket);
			}
		}
	}
	std::signal(SIGINT, originalIntHandler);
	std::signal(SIGTERM, originalTermHandler);
	// KILL ALL KIDS
	/*for (std::pair<const fd, Server>& a : sockToServer) {*/
	/*	Server& s = a.second;*/
	/*	if (s.hasCallback.empty()) continue ;*/
	/*	for (std::pair<const int, Process> p : s.hasCallback) kill(p.second.pid, SIGKILL);*/
	/*}*/
	closeMap(listeners);
	close(pollfd);
	INFO(+ std::to_string(message_count) + " messages processed");
	return (server_rv);
}
