/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   defaultpage.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/12 18:54:43 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/12 20:25:28 by mde-beer            ########   odam.nl   */
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

#include "HTTPMessage.hpp"
#include <defaultpage.hpp>
#include <filesystem>


namespace defaultpage {
	extern const std::string	pageDefaultHeader
	=	"<!DOCTYPE html>\n<html>\n"
		"\t<title>Error page</title>\n"
		"\t<body>\n";

	extern const std::string	pageDefaultFooter
	=	"\t</body>\n"
		"</html>\n";
	extern const std::map<int,HTTPMessage>	codePages = {
		{400, HTTPMessage("HTTP/1.1 400 Bad request",
				{
					"Content-Length:" + std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 63),
					"Content-Type: text/html",
					"Server: webserv",
				},
				pageDefaultHeader +
				"\t\t<h1>Error 400</h1>\n\t\toopsie poopsie, you did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
		{403, HTTPMessage("HTTP/1.1 403 Forbidden",
				{
					"Content-Length:" + std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 63),
					"Content-Type: text/html",
					"Server: webserv",
				},
				pageDefaultHeader +
				"\t\t<h1>Error 403</h1>\n\t\toopsie poopsie, you did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
		{404, HTTPMessage("HTTP/1.1 404 Not found",
				{
					"Content-Length:" + std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 63),
					"Content-Type: text/html",
					"Server: webserv",
				},
				pageDefaultHeader +
				"\t\t<h1>Error 404</h1>\n\t\toopsie poopsie, you did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
		{405, HTTPMessage("HTTP/1.1 405 Method not allowed",
				{
					"Content-Length:" + std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 63),
					"Content-Type: text/html",
					"Server: webserv",
				},
				pageDefaultHeader +
				"\t\t<h1>Error 405</h1>\n\t\toopsie poopsie, you did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
		{500, HTTPMessage("HTTP/1.1 500 Internal server error",
				{
					"Content-Length:" + std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 62),
					"Content-Type: text/html",
					"Server: webserv",
				},
				pageDefaultHeader +
				"\t\t<h1>Error 500</h1>\n\t\toopsie poopsie, we did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
	};
	auto	create200(const std::string& s) -> HTTPMessage {
		return HTTPMessage(
			"HTTP/1.1 200 OK",
			{
				"Content-Length:" + std::to_string(s.length()),
				"Content-Type: text/html",
				"Server: webserv",
			},
			s
		);
	}
	auto	create201(const HTTPMessage& m, const std::filesystem::path& path) -> HTTPMessage {
		std::string	locationUri = m.getFields().at("Host") + path.string();
		return HTTPMessage(
				"HTTP/1.1 201 Created",
				{
				"Location: " + locationUri,
				},
				""
				);
	}
}
