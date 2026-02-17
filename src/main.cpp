/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   main.cpp                                                :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/16 19:03:36 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/16 19:25:24 by mde-beer            ########   odam.nl   */
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

#include <iostream>
#include <iomanip>
#include <Parser.hpp>

#ifndef FUN_ALLOWED
# define FUN_ALLOWED false
#endif

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

#define testParser(parser, testCase) _testParser(parser, #parser, testCase)

// used a bunch throughout, TODO: move or inline
std::function<std::string(char)>	singleton = [](char c) -> std::string {
	std::string	out;
	return out + c;
};

// http business parsing, in no particular order
extern const Parser<char>			tchar = Parse::parseAnyOf({'!', '#', '$', '%', '&', '\\', '*', '+', '-', '.', '^', '_', '`', '|', '~'}) | Parse::parsePredicate(isdigit) | Parse::parsePredicate(isalpha);
extern const Parser<char>			vchar = Parse::parsePredicate([](unsigned char c) -> bool { return (c >= 0x21 && 0x7E >= c) ? true : false; });
extern const Parser<char>			obsText = Parse::parsePredicate([](unsigned char c) -> bool { return (c >= 0x80 && 0xFF >= c) ? true : false; });
extern const Parser<char>			fieldVchar = vchar | obsText;
extern const Parser<std::string>	ows = Parse::many(Parse::parseAnyOf({' ', '\t'}));
extern const Parser<std::string>	rws = Parse::some(Parse::parseAnyOf({' ', '\t'}));
extern const Parser<std::string>	crlf = Parse::parseString("\r\n");
extern const Parser<std::string>	token = Parse::some(tchar);
extern const Parser<std::string>	method = token;
std::function<std::function<std::string(std::string)>(char)>	fieldContentHelper = [](char c) -> std::function<std::string(std::string)> {
	return [c](std::string s) { return c + s; };
};
extern const Parser<std::string>	fieldContent = (fieldContentHelper >> fieldVchar) * (Parse::parseOpt<std::string>("", Parser<std::string>([](const std::string& s) noexcept -> Maybe<Pair<std::string,std::string>> {
			Maybe<Pair<std::string,std::string>>	greedy = Parse::some(Parse::parseAnyOf({' ', '\t'}) | fieldVchar)(s);
			if (!greedy.has_value()) return std::nullopt;
			std::string	reversedx = greedy.value().second;
			std::reverse(reversedx.begin(), reversedx.end());
			Maybe<Pair<std::string,std::string>>	ungreed = Parse::many(Parse::parseAnyOf({' ', '\t'}))(reversedx); // many _never_ returns error
			std::string	reversedgot = ungreed.value().second;
			std::string	reverseddropped = ungreed.value().first;
			std::reverse(reversedgot.begin(), reversedgot.end());
			std::reverse(reverseddropped.begin(), reverseddropped.end());
			return std::make_pair(reversedgot + greedy.value().first, reverseddropped);
})));
extern const Parser<std::string>	messageBody = Parse::many(Parse::parseAny());

auto	main([[maybe_unused]] int ac, [[maybe_unused]] char **av) -> int {
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
	Parser<std::string> psps = Parse::many(Parse::parseString("ps"));
	Parser<std::string>	reqpp = Parse::some(Parse::parseChar('p'));
	Parser<std::string> reqpsps = Parse::some(Parse::parseString("ps"));

	if (FUN_ALLOWED)
		std::cout << "LETS FUCKING GOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO!!!!\n";
	else
		std::cout << "Hello, World!\n";
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
	std::cout << "testing fieldcontenthelper..." << std::endl;
	auto	foo = fieldContentHelper('p');
	std::cout << "function instantiated..." << std::endl;
	std::string	out = foo("cum");
	std::cout << "result created, no errors" << std::endl;
	auto	bar = fieldContentHelper >> fieldVchar;
	std::cout << "fmap succeeded..." << std::endl;
	auto	baz = bar("cum");
	std::cout << "parsed..." << std::endl;
	auto	baaz = baz.value().second("um");
	std::cout << "combined..." << std::endl;
	testParser(fieldContent, "foobarbaz ");
	testParser(fieldContent, "foobarbaz foo");
	testParser(fieldContent, "foo  barbaz foo");
	testParser(fieldContent, "   foo  barbaz foo");
	return (0);
}
