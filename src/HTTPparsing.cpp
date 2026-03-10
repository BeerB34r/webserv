/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   HTTPparsing.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/18 15:40:43 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/18 21:43:34 by mde-beer            ########   odam.nl   */
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
#include <HTTPparsing.hpp>

namespace HTTPparsing {
	using namespace Parse;
	template <typename T>
	using Fn = std::function<T>; // rust mentioned???????
	using String = std::string;

	// what if we made functions be variables instead of functions :flushed:
	static Fn<String(char)>	singleton = [](char c) noexcept -> String {
		String	out;
		return out + c;
	};

	static Fn<String(std::vector<String>)>	concat = [](std::vector<String> v) noexcept -> String {
		String	out;
		for (std::string s : v) out.append(s);
		return out;
	};
	static Fn<bool(char)>	ishexdigit = [](char c) noexcept -> bool {
		const static std::string hexdigits = "0123456789ABCDEF"; // case sensitive, cuz so is the RFC
		return hexdigits.contains(c);
	};

	// haskell programmers be like:
	// look at what they need to mimic a fraction of our power
	static Fn<Fn<String(String)>(char)>	prependChar = [](char prefix) noexcept -> Fn<String(String)> {
		return [prefix](String postfix) { return prefix + postfix; };
	};
	static Fn<Fn<String(String)>(String)>	prependString = [](String prefix) noexcept -> Fn<String(String)> {
		return [prefix](String postfix) { return prefix + postfix; };
	};

	// http business parsing, in no particular order
	extern const Parser<char>	tchar
		= parseAnyOf({'!', '#', '$', '%', '&', '\\', '*', '+', '-', '.', '^', '_', '`', '|', '~'})
		| parsePredicate(isdigit)
		| parsePredicate(isalpha);
	extern const Parser<char>	vchar = parsePredicate([](unsigned char c) -> bool { return (c >= 0x21 && 0x7E >= c) ? true : false; });
	extern const Parser<char>	obsText = parsePredicate([](unsigned char c) -> bool { return (c >= 0x80 && 0xFF >= c) ? true : false; });
	extern const Parser<char>	fieldVchar = vchar | obsText;
	extern const Parser<String>	ows = many(parseAnyOf({' ', '\t'})); // optional whitespace
	extern const Parser<String>	rws = some(parseAnyOf({' ', '\t'})); // required whitespace
	extern const Parser<String>	crlf = parseString("\r\n");
	extern const Parser<String>	token = some(tchar);
	extern const Parser<String>	pctEncoded
		= (prependChar >> parseChar('%'))
		* ((prependChar >> parsePredicate(ishexdigit))
		* (singleton >> parsePredicate(ishexdigit)));
	extern const Parser<char>	subDelims = parseAnyOf({'!', '$', '&', '\\', '(', ')', '*', '+', ',', ':', '='});
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
	}))); // hell function
	extern const Parser<String>	fieldValue = concat >> many(fieldContent);
	extern const Parser<String>	fieldName = token;
	extern const Parser<String>	fieldLine
		= (prependString >> fieldName)
		* ((prependChar >> parseChar(':'))
		* (ows > fieldValue < ows));
	extern const Parser<std::vector<String>>	fieldLines = many(fieldLine < crlf);
	extern const Parser<String>	httpVersion
		= (prependString >> parseString("HTTP"))
		* ((prependChar >> parseChar('/'))
		* ((prependChar >> parsePredicate(isdigit))
		* ((prependChar >> parseChar('.'))
		* (singleton >> parsePredicate(isdigit)))));
	extern const Parser<char>	unreserved
		= parsePredicate(isalpha)
		| parsePredicate(isdigit)
		| parseAnyOf({'-', '.', '_', '~'});
	extern const Parser<String>	pchar
		= (singleton >> unreserved)
		| pctEncoded
		| (singleton >> subDelims)
		| (singleton >> parseChar(':'))
		| (singleton >> parseChar('@'));
	extern const Parser<String>	query
		= concat >> many(
				pchar
				| singleton >> parseChar('/')
				| singleton >> parseChar('?')
			);
	extern const Parser<String>	segmentNz = concat >> some(pchar);
	extern const Parser<String>	segment = concat >> many(pchar);
	extern const Parser<String>	pathEmpty = pure<String>("");
	extern const Parser<String>	pathRootless
		= (prependString >> segmentNz)
		* (concat >> many(
					(prependChar >> parseChar('/'))
					* segment
		));
	extern const Parser<String>	pathAbsolute
		= (prependChar >> parseChar('/'))
		* parseOpt<String>("", pathRootless);
	extern const Parser<String>	pathAbempty
		= concat >> many(
				(prependChar >> parseChar('/'))
				* segment
		);
	extern const Parser<String> decOctet
		= (prependChar >> parseChar('1')) * ((prependChar >> parsePredicate(isdigit)) * (singleton >> parsePredicate(isdigit)))
		| (prependChar >> parseChar('2')) * ((prependChar >> parseAnyOf("01234")) * (singleton >> parsePredicate(isdigit)))
		| (prependString >> parseString("25")) * (singleton >> parseAnyOf("012345"))
		| (prependChar >> parseAnyOf("123456789")) * (singleton >> parsePredicate(isdigit))
		| singleton >> parsePredicate(isdigit);
	extern const Parser<String> h16([](const String& s) -> Maybe<Pair<String,String>> {
			Maybe<Pair<String,String>> inner = some(parsePredicate(ishexdigit))(s);
			if (!inner.has_value()) return std::nullopt;
			if (inner.value().second.size() > 4) return std::nullopt;
			return inner;
		});
	extern const Parser<String>	ipv4address
		= (prependString >> decOctet)
		* ((prependChar >> parseChar('.'))
		* ((prependString >> decOctet)
		* ((prependChar >> parseChar('.'))
		* ((prependString >> decOctet)
		* ((prependChar >> parseChar('.'))
		* (decOctet))))));
	extern const Parser<String>	ls32
		= (prependString >> h16) * ((prependChar >> parseChar(':')) * h16)
		| ipv4address;
	extern const Parser<String>	port = many(parsePredicate(ishexdigit));
	extern const Parser<String>	regName
		= concat >> many(
				singleton >> unreserved
				| pctEncoded
				| singleton >> subDelims
		);
	extern const Parser<String> ipvfuture
		= (prependChar >> parseChar('v'))
		* ((prependString >> some(parsePredicate(ishexdigit)))
		* ((prependChar >> parseChar('.'))
		* some(
			unreserved
			| subDelims
			| parseChar(':')
		)));
	const Parser<String> h16c = (prependString >> h16) * (singleton >> parseChar(':'));
	extern const Parser<String>	ipv6address
		= (prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ls32)))))
		| (prependString >> parseString("::"))
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ls32)))))
		| (prependString >> parseOpt<String>("", h16))
			* ((prependString >> parseString("::"))
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ls32)))))
		|		(prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * parseString(":"))
				| (prependString >> ((prependString >> h16c) * parseString(":")))
				| (prependString >> parseString("::")))
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ls32)))
		|		(
				(prependString >> (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * parseString(":")))
				| (prependString >> ((prependString >> h16c) * parseString(":")))
				| (prependString >> parseString("::"))
				)
			* ((prependString >> h16c)
			* ((prependString >> h16c)
			* ls32))
		|		(
				(prependString >> (prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * parseString(":")))
				| (prependString >> ((prependString >> h16c) * parseString(":")))
				| (prependString >> parseString("::"))
				)
			* ((prependString >> h16c)
			* ls32)
		|		(
				(prependString >> (prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> (prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * parseString(":")))
				| (prependString >> ((prependString >> h16c) * parseString(":")))
				| (prependString >> parseString("::"))
				)
			* ls32
		|		(
				(prependString >> (prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> (prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> (prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * parseString(":"))
				| (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * parseString(":")))
				| (prependString >> ((prependString >> h16c) * parseString(":")))
				| (prependString >> parseString("::"))
				)
			* h16
		|		(prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * h16c)) * h16c)) * h16c)) * parseString(":")
				| (prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * h16c)) * h16c)) * parseString(":")
				| (prependString >> ((prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * h16c)) * parseString(":")
				| (prependString >> ((prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * h16c)) * parseString(":")
				| (prependString >> ((prependString >> ((prependString >> h16c) * h16c)) * h16c)) * parseString(":")
				| (prependString >> ((prependString >> h16c) * h16c)) * parseString(":")
				| (prependString >> h16c) * parseString(":")
				| parseString("::");
	extern const Parser<String>	ipLiteral
		= (prependChar >> parseChar('['))
		* ((prependString >> (ipv6address | ipvfuture))
		* (singleton >> parseChar(']')));
	extern const Parser<String>	host
		= ipLiteral
		| ipv4address
		| regName;
	extern const Parser<String>	uriHost = host;
	extern const Parser<String>	userinfo = concat >> many(singleton >> unreserved | pctEncoded | singleton >> subDelims | singleton >> parseChar(':'));
	extern const Parser<String>	authority
		= (prependString >> parseOpt<String>("", (prependString >> userinfo) * (singleton >> parseChar('@'))))
		* ((prependString >> host)
		* parseOpt<String>("", (prependChar >> parseChar(':')) * port));
	extern const Parser<String>	hierPart
		= ((prependString >> parseString("//")) * ((prependString >> authority) * pathAbempty))
		| pathAbsolute
		| pathRootless
		| pathEmpty;
	extern const Parser<String>	scheme = (prependChar >> parsePredicate(isalpha)) * many(parsePredicate(isalpha) | parsePredicate(isdigit) | parseAnyOf({'+', '-', '.'}));
	extern const Parser<String>	absoluteURI = (prependString >> scheme) * ((prependChar >> parseChar(':')) * ((prependString >> hierPart) * parseOpt<String>("", (prependChar >> parseChar('?')) * query)));
	extern const Parser<String>	absolutePath = concat >> some((prependChar >> parseChar('/')) * segment);
	extern const Parser<String>	reasonPhrase = some(parseAnyOf({'\t', ' '}) | vchar | obsText);
	extern const Parser<String>	statusCode = (prependChar >> parsePredicate(isdigit)) * ((prependChar >> parsePredicate(isdigit)) * (singleton >> parsePredicate(isdigit)));
	extern const Parser<String>	statusLine = (prependString >> httpVersion) * ((prependChar >> parseChar(' ')) * ((prependString >> statusCode) * ((prependChar >> parseChar(' ')) * parseOpt<String>("", reasonPhrase))));
	extern const Parser<String>	asteriskForm = parseString("*");
	extern const Parser<String>	authorityForm = (prependString >> uriHost) * ((prependChar >> parseChar(':')) * port);
	extern const Parser<String>	absoluteForm = absoluteURI;
	extern const Parser<String> originForm = (prependString >> absolutePath) * parseOpt<String>("", (prependChar >> parseChar(':')) * query);
	extern const Parser<String>	requestTarget
		= originForm
		| absoluteForm
		| authorityForm
		| asteriskForm;
	extern const Parser<String>	requestLine
		= (prependString >> method) * ((prependChar >> parseChar(' '))
		* ((prependString >> requestTarget) * ((prependChar >> parseChar(' '))
		* httpVersion)));
	extern const Parser<String>	startLine = requestLine | statusLine;
	extern const Parser<String>	messageBody = many(parseAny());
} // namespace HTTP
