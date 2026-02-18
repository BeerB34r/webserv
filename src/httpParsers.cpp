/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   httpParsers.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/18 15:40:43 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/18 16:30:17 by mde-beer            ########   odam.nl   */
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
	using namespace Parse;
	template <typename T>
	using Fn = std::function<T>; // rust mentioned???????
	using String = std::string;

	Fn<String(char)>	singleton = [](char c) -> String {
		String	out;
		return out + c;
	};
	// haskell programmers be like:
	// look at what they need to mimic a fraction of our power
	Fn<Fn<String(String)>(char)>	prependChar = [](char prefix) -> Fn<String(String)> {
		return [prefix](String postfix) { return prefix + postfix; };
	};
	Fn<Fn<String(String)>(String)>	prependString = [](String prefix) -> Fn<String(String)> {
		return [prefix](String postfix) { return prefix + postfix; };
	};

	// http business parsing, in no particular order
	extern const Parser<char>	tchar = parseAnyOf({'!', '#', '$', '%', '&', '\\', '*', '+', '-', '.', '^', '_', '`', '|', '~'}) | parsePredicate(isdigit) | parsePredicate(isalpha);
	extern const Parser<char>	vchar = parsePredicate([](unsigned char c) -> bool { return (c >= 0x21 && 0x7E >= c) ? true : false; });
	extern const Parser<char>	obsText = parsePredicate([](unsigned char c) -> bool { return (c >= 0x80 && 0xFF >= c) ? true : false; });
	extern const Parser<char>	fieldVchar = vchar | obsText;
	extern const Parser<String>	ows = many(parseAnyOf({' ', '\t'}));
	extern const Parser<String>	rws = some(parseAnyOf({' ', '\t'}));
	extern const Parser<String>	crlf = parseString("\r\n");
	extern const Parser<String>	token = some(tchar);
	extern const Parser<String>	method = token;
	extern const Parser<String>	fieldContent = (prependChar >> fieldVchar) * (parseOpt<String>("", Parser<String>([](const String& s) noexcept -> Maybe<Pair<String,String>> {
				Maybe<Pair<String,String>>	greedy = some(parseAnyOf({' ', '\t'}) | fieldVchar)(s);
				if (!greedy.has_value()) return std::nullopt;
				String	reversedx = greedy.value().second;
				std::reverse(reversedx.begin(), reversedx.end());
				Maybe<Pair<String,String>>	ungreed = many(parseAnyOf({' ', '\t'}))(reversedx); // many _never_ returns error
				String	reversedgot = ungreed.value().second;
				String	reverseddropped = ungreed.value().first;
				std::reverse(reversedgot.begin(), reversedgot.end());
				std::reverse(reverseddropped.begin(), reverseddropped.end());
				return std::make_pair(reversedgot + greedy.value().first, reverseddropped);
	})));
	extern const Parser<String>	fieldValue = many(fieldContent);
	extern const Parser<String>	fieldName = token;
	extern const Parser<String>	messageBody = many(parseAny());
} // namespace HTTP
