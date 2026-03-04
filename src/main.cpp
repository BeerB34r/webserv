/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   main.cpp                                                :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/16 19:03:36 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/19 16:49:40 by mde-beer            ########   odam.nl   */
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

#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <Parser.hpp>
#include <HTTPMessage.hpp>
#include <HTTPparsing.hpp>

std::chrono::duration<double,std::micro>	stopwatch(std::function<void()> fn) {
	std::chrono::time_point start = std::chrono::high_resolution_clock::now();
	fn();
	std::chrono::time_point end = std::chrono::high_resolution_clock::now();
	return (end - start);
}

template <typename T>
concept printable = requires (T a, std::ostream os) {
	{ os << a };
};

template <printable T>
auto	_testParser(Parser<T>	parser, const std::string& name, const std::string& testCase) -> void {
	Maybe<std::pair<std::string,T>>	result = parser(testCase);

	std::cout << "parser "
		<< std::setw(10) << std::setiosflags(std::iostream::left) << name << std::setw(0)
		<< " parsing \"" << std::setw(30) << testCase + "\"" << std::setw(0) << " returning -> ";
	if (result.has_value()) {
		T			parsed = result.value().second;
		std::string	remainder = result.value().first;
		std::cout << "Just (\"" << remainder << "\", \"";
		std::cout << parsed << "\")";
	}
	else
		std::cout << "Nothing";
	std::cout << "\n";
}
std::function<std::string(std::vector<std::string>)>	concat = [](std::vector<std::string> v) noexcept -> std::string {
	std::string	out;
	for (std::string s : v) out.append(s);
	return out;
};

#define testParser(parser, testCase) _testParser(parser, #parser, testCase)

auto	testipv6(void) -> void {
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::FFFF:FFFF:FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF::FFFF:FFFF:FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::FFFF:FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF::FFFF:FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF::FFFF:FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF::FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF::FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF::FFFF:FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF::FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF::FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF::FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF::FFFF:FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF::FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF::FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF::FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF::FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF:FFFF::FFFF:FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF::FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF::FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF::FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF::FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF:FFFF::FFFFend of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF::FFFFend of string");
	testParser(HTTPparsing::ipv6address, "::end of string");
	testParser(HTTPparsing::ipv6address, "FFFF::end of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF::end of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF::end of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF::end of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF:FFFF::end of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF::end of string");
	testParser(HTTPparsing::ipv6address, "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF::end of string");
}

auto	readhttp(std::string file) -> void {
	std::ifstream	filestream(file);
	std::ostringstream	oss;
	oss << filestream.rdbuf();
	std::string	input = oss.str();
	Maybe<HTTPMessage>	message = readHTTPmessage(input);
	if (!message) std::cout << "Nothing\n";
	else {
		std::cout << "Just (\n" << message->prettyPrint() << ")\n";
		std::cout << "machine readable:\n" << *message << "\n\n";
	}
}

auto	testParserFeatures(void) -> void {
	Parser<char>		any = Parse::parseAny();
	Parser<char>		Hs	= Parse::parseChar('H');
	Parser<char>		hs	= Parse::parseChar('H');
	std::function<std::string(char)>	singleton = [](char c) -> std::string {
		std::string	out(1, c);
		return out;
	};
	Parser<std::string>	Hstring = singleton >> Hs;
	Parser<std::string>	Truep = Parse::parseString("True");
	Parser<std::string>	Falsep = Parse::parseString("False");
	Parser<std::string>	boolp = Truep | Falsep;
	Parser<std::string>	pp = Parse::many(Parse::parseChar('p')); 
	Parser<std::string>	psps = concat >> Parse::many(Parse::parseString("ps"));
	Parser<std::string>	reqpp = Parse::some(Parse::parseChar('p'));
	Parser<std::string>	reqpsps = concat >> Parse::some(Parse::parseString("ps"));

	Parser<std::string>	scrubWs = HTTPparsing::ows > HTTPparsing::token < HTTPparsing::ows;

	testParser(any, "Hello, World!");
	testParser(Hs, "Hello, World!");
	testParser(Hstring, "Hello, World!");
	testParser(hs, "Hello, World!");
	testParser(any, "foo bar baz");
	testParser(Hstring, "foo bar baz");
	testParser(hs, "foo bar baz");
	testParser(Truep, "True, and thats True");
	testParser(Falsep, "True, and thats True");
	testParser(boolp, "True, and thats True");
	testParser(Truep, "False, and thats humbug");
	testParser(Falsep, "False, and thats humbug");
	testParser(boolp, "False, and thats humbug");
	testParser(pp, "pppppfoo");
	testParser(pp, "pfoo");
	testParser(pp, "foo");
	testParser(psps, "pspspspspsfoo");
	testParser(psps, "psfoo");
	testParser(psps, "foo");
	testParser(reqpp, "pppppfoo");
	testParser(reqpp, "pfoo");
	testParser(reqpp, "foo");
	testParser(reqpsps, "pspspspspsfoo");
	testParser(reqpsps, "psfoo");
	testParser(reqpsps, "foo");
	testParser(HTTPparsing::fieldContent, "foobarbaz ");
	testParser(HTTPparsing::fieldContent, "foobarbaz foo");
	testParser(HTTPparsing::fieldContent, "foo  barbaz foo   \tf\t");
	testParser(HTTPparsing::fieldContent, "   foo  barbaz foo");
	testParser(scrubWs, "  bar  baz");
	testParser(scrubWs, "bar  baz  ");
	testParser(scrubWs, "bar");
}

auto inline	testHTTPparsingfile(const std::string& fname) {
	std::cout << "time to parse: " << fname << "\n";
	std::cout << "\n" << stopwatch([fname](){ readhttp(fname); }) <<  "\n";
}

bool	g_funAllowed = false;

auto	testHTTPparsing(void) -> void {
	testHTTPparsingfile("hs/response.http");
	testHTTPparsingfile("hs/request.http");
}

auto	main([[maybe_unused]] int ac, [[maybe_unused]] char **av) -> int {

	g_funAllowed = (std::string(0[av]) == "./ginxnay") || (std::string(0[av]) == "ginxnay");
	if (g_funAllowed)
		std::cout << "LETS FUCKING GOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO!!!!\n";
	else
		std::cout << "Hello, World!\n";
	testHTTPparsing();
	return (0);
}
