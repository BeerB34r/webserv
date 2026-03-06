/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   server.cpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/06 16:08:37 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/06 16:10:28 by mde-beer            ########   odam.nl   */
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

#include <server.hpp>

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <HTTPMessage.hpp>

#define PORT 8080
#define LISTEN_BACKLOG 50
#define BUFFER_SIZE 1024

auto	mvpServer(void) -> int {
	int	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		std::cerr << "failed to create socket\n";
		return (1);
	}
	struct sockaddr_in	addr = {};

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr))) {
		perror(strerror(errno));
		std::cerr << "failed to bind socket\n";
		return (1);
	}
	if (listen(sock, LISTEN_BACKLOG) == -1) {
		std::cerr << "sockets killed my grandma ok?\n";
		return (1);
	}

	struct sockaddr_in	peer;
	socklen_t	peerAddrSize = sizeof(peer);
	int	peerFD;

	while (true) {
		peerFD = accept(sock, reinterpret_cast<struct sockaddr*>(&peer), &peerAddrSize);
		if (peerFD < 0) {
			std::cerr << "failed to accept incoming traffic\n";
			return (1);
		}

		char	buf[BUFFER_SIZE];
		int	bytes;
		std::string	in;
		Maybe<HTTPMessage>	message;
		do  {
			bytes = read(peerFD, buf, BUFFER_SIZE);
			in.append(buf, bytes);
			message = readHTTPmessage(in);
			if (message) {
				std::cout << message->prettyPrint() << "\n";
			} else if (bytes != BUFFER_SIZE) {
				std::cerr << "incoming traffic not recognised as httprequest\n";
			}
		} while (bytes == BUFFER_SIZE);
		if (message)
			write(peerFD, "HTTP/1.1 404 Not found\r\n\r\n", 26);
		else
			write(peerFD, "HTTP/1.1 400 Bad request\r\n\r\n", 28);
		close(peerFD);
		if (bytes < 0) {
			std::cerr << "read error\n";
			return (1);
		}
	}
	close(sock);
	return (0);
}
