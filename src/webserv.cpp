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

#include <cassert>
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

#define _evs(s, e, m) do { \
	if (e & m) \
		s.empty() ? s = #m : s += "|"s + #m; \
} while (0)

static inline auto	evToString(int e) -> std::string {
	std::string	rv = "";
	_evs(rv, e, EPOLLIN);
	_evs(rv, e, EPOLLPRI);
	_evs(rv, e, EPOLLOUT);
	_evs(rv, e, EPOLLRDNORM);
	_evs(rv, e, EPOLLRDBAND);
	_evs(rv, e, EPOLLWRNORM);
	_evs(rv, e, EPOLLWRBAND);
	_evs(rv, e, EPOLLMSG);
	_evs(rv, e, EPOLLERR);
	_evs(rv, e, EPOLLHUP);
	_evs(rv, e, EPOLLRDHUP);
	_evs(rv, e, EPOLLEXCLUSIVE);
	_evs(rv, e, EPOLLWAKEUP);
	_evs(rv, e, EPOLLONESHOT);
	_evs(rv, e, EPOLLET);
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

static auto	fulfillGetRequest(const Server& self, const HTTPMessage& http, const std::string& request, const std::filesystem::path& target, const std::string& query, struct in_addr peer_addr) -> std::variant<Server::Cgi,HTTPMessage> {
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

auto	fulfillRequestTarget(const Server& self, HTTPMessage http, const std::string& request, const std::filesystem::path& target, const std::string& query, struct in_addr peer_addr) -> std::variant<Server::Cgi,HTTPMessage> {
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

auto	fulfillRequest(Server& self, struct epoll_event* ev, struct in_addr peer_addr) noexcept -> std::variant<Server::Cgi,HTTPMessage> {
	Server::Client&	client = self.clients[ev->data.fd];
	std::optional<HTTPMessage>	http = readHTTPrequest(client.data);
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
	Server::Client&	client = self.clients[ev->data.fd];
	bool	should_close = false;
	if (client.cgi) {
		client.cgi->clientReady = true;
		return false;
	}
	if (!client.response) {
		ERROR("what the fuck");
		return false;
	}
	HTTPMessage	message = *client.response;
	int	status = std::get<HTTPMessage::ResponseData>(message.getData()).statusCode;
	if (status > 399 || 200 > status) should_close = true;
	std::stringstream	ss;
	ss << message;
	std::string			response = ss.str();
	send(client.sock, response.c_str(), response.length(), 0);
	client.data = "";
	client.response = std::nullopt;
	if (should_close) {
		epoll_ctl(pollfd, EPOLL_CTL_DEL, client.sock, ev);
		close(client.sock);
	} else {
		ev->events = EPOLLIN | EPOLLET;
		epoll_ctl(pollfd, EPOLL_CTL_MOD, client.sock, ev);
	}
	INFO("request from port " + std::to_string(self.port) + " fulfilled");
	return (should_close);
}

auto	defaultReadEventHandler(Server& self, int pollfd, struct epoll_event* ev, struct in_addr peer_addr) -> bool {
	Server::Client&	client = self.clients[ev->data.fd];
	std::array<char, BUFFER_SIZE>	buf;
	int rv;
	size_t	totalRead = 0;
	std::optional<HTTPMessage>	http = std::nullopt;
	while ((rv = recv(ev->data.fd, buf.data(), buf.max_size(), MSG_DONTWAIT)) > 0) {
		client.data.append(buf.data(), rv);
		totalRead += rv;
		if (totalRead > self.maxRequestSize) break ;
		http = readHTTPrequest(client.data); // inefficient, but like, lets benchmark it first lmao // its fine
		if (http && http->getBody().size() > self.maxBodySize) break ;
	}
	if (rv < 0) { // assume error is EAGAIN/EWOULDBLOCK, not allowed to check cuz fuck you
		INFO("end of current message from port " + std::to_string(self.port));
	}
	if (totalRead > self.maxRequestSize || (http && http->getBody().size() > self.maxBodySize)) client.response = self.statusPages.at(413);
	else {
		// varaint containing either:
		// A => HTTP response
		// B => callback that returns a response, and the read-end of a pipe containing a CGI process (for epoll)
		std::variant<Server::Cgi,HTTPMessage>	requestResult = fulfillRequest(self, ev, peer_addr);
		if (std::holds_alternative<HTTPMessage>(requestResult)) client.response = std::get<HTTPMessage>(requestResult);
		else {
			Server::Cgi cgi = std::get<Server::Cgi>(requestResult);
			client.cgi = cgi;
		}
	}
	if (rv == 0) { // end of communication
		INFO("connection closed on port " + std::to_string(self.port));
		close(ev->data.fd);
		self.clients.erase(ev->data.fd);
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

static inline auto	getServer(std::map<int,Server>&	servers, int fd) noexcept -> Server& {
	for (auto& [ _, server ] : servers) {
		if (server.clients.contains(fd)) return server;
	}
	for (auto& [ _, server ] : servers) for (auto& [ _, client ] : server.clients) {
		if (!client.cgi) continue ;
		if (client.cgi->in == fd || client.cgi->out == fd) return server;
	}
	std::unreachable();
}

static inline auto	getClient(Server& server, int fd) noexcept -> Server::Client& {
	for (auto& [ _, client] : server.clients) {
		if (!client.cgi) continue ;
		if (client.cgi->in == fd || client.cgi->out == fd) return client;
	}
	std::unreachable();
}

static auto	closeMap(std::map<int,Server>&	fds) noexcept -> void {
	for (auto fd : fds) close(fd.first);
}
static auto	closeSet(std::set<int>&	fds) noexcept -> void {
	for (auto fd : fds) close(fd);
}

static inline auto	collapseEvents(int e) {
	int	shift = 0;
	while (e >> (shift + 1)) ++shift;
	return (e >> shift) << shift;
}

static inline auto	addNewClient(struct epoll_event ev, int pollfd, std::map<int,Server>& listeners, std::set<int>& clients) -> bool {
	using fd = int;
	fd	socket = ev.data.fd;
	struct in_addr peer_addr;
	fd	connection_socket = getIncomingConnection(socket, peer_addr);
	if (connection_socket < 0) return false;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = connection_socket;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, connection_socket, &ev) < 0) {
		FATAL("failed to add incoming connection to epoll instance");
		close(connection_socket); // only know you love her when you let her go
		return true;
	}
	listeners[socket].clients.insert_or_assign(connection_socket, Server::Client({
			.sock = connection_socket,
			.addr = peer_addr
			}));
	clients.insert(connection_socket);
	INFO("opened connection on port " + std::to_string(listeners[socket].port));
	return false;
}

static inline auto	handleServerEvent(struct epoll_event *current, int pollfd, std::map<int,Server>& listeners, std::set<int>& clients, std::set<int>& procs) -> bool {
	int	events = current->events;
	(void)procs;
	switch (collapseEvents(events)) {
		case (EPOLLERR): {
			return false ;
		}
		case (EPOLLHUP): {
			return false ;
		}
		case (EPOLLIN): {
			if (addNewClient(*current, pollfd, listeners, clients)) return true;
			return false ;
		}
		default: {
			WARN("unhandled event on server socket: " + evToString(current->events));
			return false ;
		}
	}
}

static inline auto	handleClientEvent(struct epoll_event *current, int pollfd, int& message_count, Server& server, std::set<int>& clients, std::set<int>& procs) -> void {
	using fd = int;
	fd	socket = current->data.fd;
	int	events = current->events;
	Server::Client&	client = server.clients.at(socket);
	switch (collapseEvents(events)) {
		case (EPOLLERR): {
			close(socket);
			if (client.cgi) {
				kill(client.cgi->pid, SIGKILL);
				close(client.cgi->in);
				close(client.cgi->out);
				procs.erase(client.cgi->in);
				procs.erase(client.cgi->out);
			}
			server.clients.erase(socket);
			clients.erase(socket);
			return ;
		}
		case (EPOLLHUP): {
			close(socket);
			if (client.cgi) {
				kill(client.cgi->pid, SIGKILL);
				close(client.cgi->in);
				close(client.cgi->out);
				procs.erase(client.cgi->in);
				procs.erase(client.cgi->out);
			}
			server.clients.erase(socket);
			clients.erase(socket);
			return ;
		}
		case (EPOLLIN): {
			if (defaultReadEventHandler(server, pollfd, current, client.addr)) {
				server.clients.erase(socket);
				clients.erase(socket);
			} else if (client.cgi) {
				struct epoll_event	ev;
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = client.cgi->in;
				if (epoll_ctl(pollfd, EPOLL_CTL_ADD, client.cgi->in, &ev) < 0) {
					close(client.cgi->in);
					close(client.cgi->out);
					client.response = server.statusPages.at(500);
					return ;
				}
				ev.events = EPOLLOUT | EPOLLET;
				ev.data.fd = client.cgi->out;
				if (epoll_ctl(pollfd, EPOLL_CTL_ADD, client.cgi->out, &ev) < 0) {
					close(client.cgi->in);
					close(client.cgi->out);
					client.response = server.statusPages.at(500);
					return ;
				}
				procs.insert(client.cgi->in);
				procs.insert(client.cgi->out);
			}
			message_count += !(current->events & EPOLLIN);
			return ;
		}
		case (EPOLLOUT): {
			if (defaultWriteEventHandler(server, pollfd, current, client.addr)) {
				server.clients.erase(socket);
				clients.erase(socket);
			}
			return ;
		}
		default: {
			WARN("unhandled event on client pipe: " + evToString(current->events));
			return ;
		}
	}
}

static inline auto	handleProcEvent(struct epoll_event *current, int pollfd, Server& server, std::set<int>& clients, std::set<int>& procs) -> void {
	using fd = int;
	fd	socket = current->data.fd;
	int	events = current->events;
	Server::Client& client = getClient(server, socket);
	struct epoll_event	ev{};
	ev.data.fd = client.sock;
	switch (collapseEvents(events)) {
		case (EPOLLERR): {
			client.response = server.statusPages.at(500);
			if (client.cgi->in >= 0) {
				close(client.cgi->in);
				procs.erase(client.cgi->in);
			}
			if (client.cgi->out >= 0) {
				close(client.cgi->out);
				procs.erase(client.cgi->out);
			}
			if (client.cgi->clientReady) if (defaultWriteEventHandler(server, pollfd, &ev, client.addr)) {
				server.clients.erase(socket);
				clients.erase(socket);
			}
			client.cgi = std::nullopt;
			return ;
		}
		case (EPOLLHUP): { // client closed their end of pipe
			if (client.cgi->in == socket) {
				close(client.cgi->in);
				procs.erase(client.cgi->in);
				client.cgi->in = -1;
				client.cgi->indata = "";
				return ;
			}
			[[fallthrough]];
		}
		case (EPOLLIN): {
			if (client.cgi->in >= 0) {
				procs.erase(client.cgi->in);
				close(client.cgi->in);
			}
			client.response = cgi::callback(client.cgi->out, server, client.cgi->pid);
			procs.erase(client.cgi->out);
			bool	ready = client.cgi->clientReady;
			client.cgi = std::nullopt;
			if (ready) if (defaultWriteEventHandler(server, pollfd, &ev, client.addr)) {
				server.clients.erase(socket);
				clients.erase(socket);
			}
			return ;
		}
		case (EPOLLOUT): {
			write(socket, client.cgi->indata.c_str(), client.cgi->indata.size());
			close(socket);
			procs.erase(socket);
			client.cgi->in = -1;
			client.cgi->indata = "";
			return ;
		}
		default: {
			WARN("unhandled event on process pipe: " + evToString(current->events));
			return ;
		}
	}
}

auto	webserv(const std::vector<Server>& servers) noexcept -> int {
	using namespace std::literals;
	using fd = int;
	int	server_rv = 0;
	struct epoll_event	ev;
	struct epoll_event	events[MAX_EVENTS];

	std::map<fd,Server>	listeners;
	{ // set up network sockets
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
	}
	std::function<Server&(fd)>	sockToServer = [&listeners](fd sock) noexcept -> Server& { return getServer(listeners, sock); };

	const fd	pollfd = epoll_create1(0); // passenger riff
	if (pollfd < 0) {
		FATAL("failed to create epoll instance");
		closeMap(listeners);
		return (1);
	}

	for (std::pair<const fd,Server> p : listeners) {// add the listening sockets into epoll
		const fd	sock = p.first;
		ev.events = EPOLLIN;
		ev.data.fd = sock;
		if (epoll_ctl(pollfd, EPOLL_CTL_ADD, sock, &ev) < 0) {
			FATAL("failed to add network socket into epoll instance");
			close(pollfd); // only miss the sun when it starts to snow
			closeMap(listeners);
			return (1);
		}
	}

	__sighandler_t	originalIntHandler = std::signal(SIGINT, intHandler);
	__sighandler_t	originalTermHandler = std::signal(SIGTERM, intHandler);
	int message_count = 0;
	std::set<fd>	clients{};
	std::set<fd>	procs{};
	while (!stop || server_rv) {
		int	nfds = epoll_wait(pollfd, events, MAX_EVENTS, 0);
		for (int n = 0; n < nfds; n++) {
			struct epoll_event	*current = &events[n];
			int	socket = current->data.fd;
			if (listeners.contains(socket)) {
				if ((server_rv = !!handleServerEvent(current, pollfd, listeners, clients, procs))) break ;
			} else if (clients.contains(socket))
				handleClientEvent(current, pollfd, message_count, sockToServer(socket), clients, procs);
			else if (procs.contains(socket)) {
				Server&			server = sockToServer(socket);
				handleProcEvent(current, pollfd, server, clients, procs);
			}
			else {
				epoll_ctl(pollfd, EPOLL_CTL_DEL, socket, current); // silently unfollow fds we dont know of
			}
		}
	}
	std::signal(SIGINT, originalIntHandler);
	std::signal(SIGTERM, originalTermHandler);
	// KILL ALL KIDS
	for (auto& s : listeners) for (auto& c : s.second.clients) {
		Server::Client&	client = c.second;
		if (!client.cgi) continue ;
		Server::Cgi&	cgi = client.cgi.value();
		kill(cgi.pid, SIGKILL);
	}
	closeSet(procs);
	closeSet(clients);
	closeMap(listeners);
	close(pollfd);
	INFO(+ std::to_string(message_count) + " messages processed");
	return (server_rv);
}
