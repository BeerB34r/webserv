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

#include <functional>
#include <server.hpp>

#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <HTTPMessage.hpp>

#define PORT 8080
#define LISTEN_BACKLOG 50
#define BUFFER_SIZE 1024

// returns negative value on error
auto	createSocket(short port) -> int {
	int	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		std::cerr << "failed to create socket\n";
		return (-1);
	}
	struct sockaddr_in	addr = {};

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int	yes = 1; // solaris???
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr))) {
		perror(strerror(errno));
		std::cerr << "failed to bind socket\n";
		close(sock);
		return (-1);
	}
	if (listen(sock, LISTEN_BACKLOG) == -1) {
		std::cerr << "failed to listen on socket\n";
		close(sock);
		return (-1);
	}
	return (sock);
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

// instance of the RequestHandler function type
auto	handleRequest(Maybe<HTTPMessage> request) -> HTTPMessage {
	if (!request) return HTTPMessage("HTTP/1.1 400 Bad request", {}, "");
	else return HTTPMessage("HTTP/1.1 404 Not found", {}, "");
}

auto	handleIncomingTraffic(int fd, RequestHandler handler) -> int {
	char	buf[BUFFER_SIZE];
	int	bytes;
	std::string	in;
	Maybe<HTTPMessage>	message;
	do {
		bytes = read(fd, buf, BUFFER_SIZE);
		if (bytes < 0) {
			std::cerr << "read error\n";
			return 1;
		}
		in.append(buf, bytes);
		message = readHTTPrequest(in);
		if (!message && bytes != BUFFER_SIZE) std::cerr << "incoming traffic not recognised as http request\n";
	} while (bytes == BUFFER_SIZE);
	std::stringstream	ss;
	ss << handler(message);
	write(fd, ss.str().c_str(), ss.str().size());
	return (0);
}

auto	mvpServer(void) -> int {
	using fd = int;
	const fd	sock = createSocket(PORT);
	if (sock < 0) return (1);

	while (true) {
		fd	peerFD = getIncomingConnection(sock);
		if (peerFD < 0) goto error;
		int	rv = handleIncomingTraffic(peerFD, handleRequest);
		close(peerFD);
		if (rv) goto error;
	}
	close(sock);
	return (0);
error: // MY WORLDS ON FIRE, HOW BOUT YOURS
	close(sock);
	return (1);
}
