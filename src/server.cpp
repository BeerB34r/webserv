/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   server.cpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/06 16:08:37 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/06 16:28:10 by mde-beer            ########   odam.nl   */
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

#include <asm-generic/socket.h>
#include <csignal>
#include <functional>
#include <map>
#include <server.hpp>

#include <iostream>
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <fcntl.h>

#include <HTTPMessage.hpp>

#define PORT 8080
#define LISTEN_BACKLOG 50
#define BUFFER_SIZE 1024

// returns negative value on error
auto	createSocket(short port) -> int {
	int	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		std::cerr << "failed to create network socket\n";
		return (-1);
	}

	struct sockaddr_in	addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int	reuse = 1;

	// allow reuse of address and port
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror(strerror(errno));
		std::cerr << "failed while setting network socket option \"reuse address\"\n";
		goto error;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		perror(strerror(errno));
		std::cerr << "failed while setting network socket option \"reuse port\"\n";
		goto error;
	}
	// do not let the port remain open after the program closes
	linger	lin;
	lin.l_linger = 0;
	lin.l_onoff = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char *>(&lin), sizeof(lin))) {
		perror(strerror(errno));
		std::cerr << "failed while setting network socket option \"linger\"\n";
		goto error;
	}

	if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr))) {
		perror(strerror(errno));
		std::cerr << "failed to bind network socket\n";
		goto error;
	}
	if (listen(sock, LISTEN_BACKLOG) == -1) {
		std::cerr << "failed to listen on network socket\n";
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
	if (out < 0) std::cerr << "failed to accept incoming traffic\n";
	return out;
}

typedef std::function<HTTPMessage(Maybe<HTTPMessage>)> RequestHandler;

bool	stop;

auto	intHandler([[maybe_unused]] int signum) -> void {
	stop = true;
}

const HTTPMessage	notFound("HTTP/1.1 404 Not found", {}, "");
const HTTPMessage	badRequest("HTTP/1.1 400 Bad request", {}, "");

#define MAX_EVENTS 10
auto	mvpServer(void) -> int {
	using namespace std::literals;
	using fd = int;

	int	server_rv = 0;

	// set up network socket
	const fd	listen_sock = createSocket(PORT);
	if (listen_sock < 0) return (1);

	// set up epoll
	struct epoll_event	events[MAX_EVENTS];
	const fd	pollfd = epoll_create1(0);
	if (pollfd < 0) {
		std::cerr << "failed to create epoll instance\n";
		close(listen_sock);
		return (1);
	}

	// add the listening socket into epoll
	struct epoll_event	ev;
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, listen_sock, &ev) < 0) {
		std::cerr << "failed to add network socket into epoll instance\n";
		close(pollfd);
		close(listen_sock);
		return (1);
	}

	__sighandler_t	originalIntHandler = signal(SIGINT, intHandler);
	int total = 0; // total connections
	std::map<int, std::string>	connection_data;
	while (!stop || server_rv) {
		int	nfds = epoll_wait(pollfd, events, MAX_EVENTS, 0);
		for (int n = 0; n < nfds; n++) {
			struct epoll_event	*current = &events[n];

			// new connection
			if (current->data.fd == listen_sock) {
				fd	connection_socket = getIncomingConnection(listen_sock);
				if (connection_socket < 0) continue ;
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = connection_socket;
				if (epoll_ctl(pollfd, EPOLL_CTL_ADD, connection_socket, &ev) < 0) {
					std::cerr << "failed to add incoming connection to epoll instance\n";
					close(connection_socket);
					server_rv = 1;
					break ;
				}
			} else if (current->events & EPOLLIN) { // new read event
				char	buf[BUFFER_SIZE];
				int	rv = recv(current->data.fd, buf, BUFFER_SIZE, 0);
				// check HTTP around here to prevent requests that are too large
				connection_data[current->data.fd].append(buf, rv);
				if (rv < 0)
					perror(strerror(errno));
				if (rv == 0)
					std::cerr << "end of file found in incoming connection\n";
				current->events = EPOLLOUT | EPOLLET;
				epoll_ctl(pollfd, EPOLL_CTL_MOD, current->data.fd, current);
			} else { // new write event
				std::stringstream	ss;
				if (readHTTPmessage(connection_data[current->data.fd])) ss << notFound;
				else ss << badRequest;
				send(current->data.fd, ss.str().c_str(), ss.str().length(), 0);
				total++;
				close(current->data.fd);
				connection_data.erase(current->data.fd);
			}
		}
	}
	signal(SIGINT, originalIntHandler);
	close(listen_sock);
	close(pollfd);
	std::cout << total << " messages processed\n";
	return (server_rv);
}
