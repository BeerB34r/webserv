/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Server.hpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/10 19:53:15 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/24 16:36:51 by mde-beer            ########   odam.nl   */
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

#ifndef SERVER_HPP
# define SERVER_HPP

#include <Config.hpp>		// Config
#include <HTTPMessage.hpp>	// HTTPMessage
#include <chrono>
#include <defaultpage.hpp>	// defaultpage::codePages
#include <filesystem>		// std::filesystem::path
#include <limits>
#include <netinet/in.h>		// INADDR_ANY

using namespace std::literals;
#define DEFAULT_TIMEOUT 30s

typedef struct	process {
	using fd = int;
	fd	in, out;
	bool	clientReady;
	fd	client;
	pid_t	pid;
	std::string	stdinContent, stdoutContent;
}	Process;

struct Server : public Config {
	using eventHandler = std::function<bool(Server&, int, struct epoll_event*, struct in_addr)>;
	short	port;
	long	address = INADDR_ANY;
	std::string	root;
	eventHandler	readEventHandler; // in practice never changed
	eventHandler	writeEventHandler;
	auto	toLocal(const std::string&) -> std::string;
	std::map<int,std::string>	client_data;
	std::map<int,HTTPMessage>	statusPages = defaultpage::codePages;
	std::map<int,HTTPMessage>	responses;
	std::map<int,Process>	hasCallback;
	std::map<std::string,std::pair<std::string,std::set<HTTPMessage::HTTPMethod>>>	routes;
	std::set<std::string>	cgiExts;
	std::set<std::string>	cgiDirs;
	std::set<std::string>	dataDirs;
	std::set<HTTPMessage::HTTPMethod>	supportedMethods;
	size_t	maxRequestSize = std::numeric_limits<size_t>::max();
	size_t	maxBodySize = std::numeric_limits<size_t>::max();
	std::chrono::seconds	cgiTimeout = DEFAULT_TIMEOUT;
};

auto	fromConfig(const Config&) -> std::vector<Server>;
auto	fulfillRequestTarget(const Server&, HTTPMessage, const std::string&, const std::filesystem::path&, const std::string&, struct in_addr) -> std::variant<std::pair<int,Process>,HTTPMessage>;

#endif // SERVER_HPP
