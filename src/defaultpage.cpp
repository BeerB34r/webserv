/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   defaultpage.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/12 18:54:43 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/12 19:26:05 by mde-beer            ########   odam.nl   */
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

#include <defaultpage.hpp>

const static std::string	pageDefaultHeader
=	"<!DOCTYPE html>\n<html>\n"
	"<title>Default error page</title>\n"
	"<body>\n";

const static std::string	pageDefaultFooter
=	"</body>\n"
	"</html>\n";

namespace defaultpage {
	extern const std::map<int,HTTPMessage>	codePages = {
		{400, HTTPMessage("HTTP/1.1 400 Bad request",
				{
					"content-length:"
					+ std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 61)
				},
				pageDefaultHeader +
				"\t<h1>Error 400</h1>\n\toopsie poopsie, you did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
		{403, HTTPMessage("HTTP/1.1 403 Forbidden",
				{
					"content-length:"
					+ std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 61)
				},
				pageDefaultHeader +
				"\t<h1>Error 403</h1>\n\toopsie poopsie, you did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
		{404, HTTPMessage("HTTP/1.1 404 Not found",
				{
					"content-length:"
					+ std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 61)
				},
				pageDefaultHeader +
				"\t<h1>Error 404</h1>\n\toopsie poopsie, you did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
		{500, HTTPMessage("HTTP/1.1 500 Internal server error",
				{
					"content-length:"
					+ std::to_string(pageDefaultFooter.size() + pageDefaultHeader.size() + 60)
				},
				pageDefaultHeader +
				"\t<h1>Error 500</h1>\n\toopsie poopsie, we did a fuckie wuckie\n"
				+ pageDefaultFooter)
		},
	};
}
