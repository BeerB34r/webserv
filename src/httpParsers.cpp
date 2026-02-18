/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   httpParsers.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/18 15:40:43 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/18 15:53:13 by mde-beer            ########   odam.nl   */
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

#include <Parser.hpp>
#include <HttpMessage.hpp>

namespace HTTP {
	template <typename T>
	using Func = std::function<T>;
	using String = std::string;

	Func<String(char)>	singleton = [](char c) -> String {
		String	out;
		return out + c;
	};

	// http business parsing, in no particular order
	extern const Parser<char>	tchar = Parse::parseAnyOf({'!', '#', '$', '%', '&', '\\', '*', '+', '-', '.', '^', '_', '`', '|', '~'}) | Parse::parsePredicate(isdigit) | Parse::parsePredicate(isalpha);
	extern const Parser<char>	vchar = Parse::parsePredicate([](unsigned char c) -> bool { return (c >= 0x21 && 0x7E >= c) ? true : false; });
	extern const Parser<char>	obsText = Parse::parsePredicate([](unsigned char c) -> bool { return (c >= 0x80 && 0xFF >= c) ? true : false; });
	extern const Parser<char>	fieldVchar = vchar | obsText;
	extern const Parser<String>	ows = Parse::many(Parse::parseAnyOf({' ', '\t'}));
	extern const Parser<String>	rws = Parse::some(Parse::parseAnyOf({' ', '\t'}));
	extern const Parser<String>	crlf = Parse::parseString("\r\n");
	extern const Parser<String>	token = Parse::some(tchar);
	extern const Parser<String>	method = token;
	Func<Func<String(String)>(char)>	fieldContentHelper = [](char c) -> Func<String(String)> {
		return [c](String s) { return c + s; };
	};
	extern const Parser<String>	fieldContent = (fieldContentHelper >> fieldVchar) * (Parse::parseOpt<String>("", Parser<String>([](const String& s) noexcept -> Maybe<Pair<String,String>> {
				Maybe<Pair<String,String>>	greedy = Parse::some(Parse::parseAnyOf({' ', '\t'}) | fieldVchar)(s);
				if (!greedy.has_value()) return std::nullopt;
				String	reversedx = greedy.value().second;
				std::reverse(reversedx.begin(), reversedx.end());
				Maybe<Pair<String,String>>	ungreed = Parse::many(Parse::parseAnyOf({' ', '\t'}))(reversedx); // many _never_ returns error
				String	reversedgot = ungreed.value().second;
				String	reverseddropped = ungreed.value().first;
				std::reverse(reversedgot.begin(), reversedgot.end());
				std::reverse(reverseddropped.begin(), reverseddropped.end());
				return std::make_pair(reversedgot + greedy.value().first, reverseddropped);
	})));
	extern const Parser<String>	messageBody = Parse::many(Parse::parseAny());
} // namespace HTTP
