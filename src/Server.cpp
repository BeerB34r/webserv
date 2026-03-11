/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Server.cpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/10 19:57:48 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/10 20:19:09 by mde-beer            ########   odam.nl   */
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
#include <charconv>
#include <vector>

#include <asm-generic/socket.h>
#include <csignal>
#include <functional>
#include <map>

#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <fcntl.h>

#include <debug.hpp>
#include <HTTPMessage.hpp>

static auto	to_int(const std::string& s) noexcept -> std::optional<int> {
	int	rv{};
	if (std::from_chars(s.data(), s.data() + s.size(), rv).ec == std::errc{}) {
		return rv;
	}
	else return std::nullopt;
}

const HTTPMessage	notFound("HTTP/1.1 404 Not found", {}, "");
const HTTPMessage	badRequest("HTTP/1.1 400 Bad request", {}, "");

#define BUFFER_SIZE 1024
auto	defaultWriteEventHandler(Server& self, [[maybe_unused]] int pollfd, struct epoll_event* ev) -> bool {
	std::stringstream	ss;
	if (readHTTPmessage(self.client_data[ev->data.fd])) ss << notFound;
	else ss << badRequest;
	send(ev->data.fd, ss.str().c_str(), ss.str().length(), 0);
	INFO("responded to connection on port " + std::to_string(self.port));
	close(ev->data.fd);
	self.client_data.erase(ev->data.fd);
	return (true);
}

auto	defaultReadEventHandler(Server& self, int pollfd, struct epoll_event* ev) -> bool {
	char	buf[BUFFER_SIZE];
	int	rv = recv(ev->data.fd, buf, BUFFER_SIZE, 0);
	// check HTTP around here to prevent requests that are too large
	self.client_data[ev->data.fd].append(buf, rv);
	if (rv < 0)
		perror(strerror(errno));
	if (rv == 0)
		INFO("end of file found in incoming connection");
	// should probably loop so we get the entire message because ET
	ev->events = EPOLLOUT | EPOLLET;
	epoll_ctl(pollfd, EPOLL_CTL_MOD, ev->data.fd, ev);
	return (false);
}

auto	fromConfig(const Config& c) -> std::vector<Server> {
	std::vector<Server>	rv;

	if (!c.contains(Config::SERVER)) {
		Server s;
		s.blocks = c.blocks;
		s.ident = c.ident;
		s.values = c.values;
		s.port = *to_int(c.values.at("listen"));
		s.writeEventHandler = defaultWriteEventHandler;
		s.readEventHandler = defaultReadEventHandler;
		rv.push_back(s);
	} else for (Config& current : c.getBlocks(Config::SERVER)) {
		Server s;
		s.blocks = current.blocks;
		s.ident = current.ident;
		s.values = current.values;
		s.port = *to_int(current.values.at("listen"));
		s.writeEventHandler = defaultWriteEventHandler;
		s.readEventHandler = defaultReadEventHandler;
		rv.push_back(s);
	}
	return rv;
}

#define PORT 8080
#define LISTEN_BACKLOG 50

// returns negative value on error
auto	createSocket(short port) -> int {
	int	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		ERROR("failed to create network socket");
		return (-1);
	}

	struct sockaddr_in	addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
		ERROR("failed to bind network socket");
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
	for (const Server& s : servers) {
		const fd	sock = createSocket(s.port);
		if (sock < 0) continue ;
		listeners[sock] = s;
	}
	if (listeners.empty()) {
		FATAL("Failed to instantiate any listening sockets");
		return (1); // cuz you only need the light when its burnin low
	}

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
			} else  { // new client event
				Server&	serverConfig = sockToServer[socket];
				if (current->events & EPOLLIN
					? message_count++, serverConfig.readEventHandler(serverConfig, pollfd, current)
					: serverConfig.writeEventHandler(serverConfig, pollfd, current)
					) // can we pretend that airplanes in the night sky are like shooting stars
					sockToServer.erase(socket);
			}
		}
	}
	signal(SIGINT, originalIntHandler);
	closeMap(listeners);
	close(pollfd);
	INFO(+ std::to_string(message_count) + " messages processed");
	return (server_rv);
}
