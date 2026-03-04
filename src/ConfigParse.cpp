/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   ConfigParse.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/04 17:06:05 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/04 19:53:21 by mde-beer            ########   odam.nl   */
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
#include <Config.hpp>
#include <iostream>
#include <variant>

namespace ConfigParse {
	using namespace Parse;
	using String = std::string;

	template <typename T>
	using Fn = std::function<T>;

	const Parser<char>	tchar
			= parseAnyOf({'!', '#', '$', '%', '&', '\\', '*', '+', '-', '.', '^', '_', '`', '|', '~'})
			| parsePredicate(isdigit)
			| parsePredicate(isalpha);

	extern const std::function<Config::Type(String)>	toType = [](const String& s) -> Config::Type {
		String	low = s;
		for (char& c : low) c = tolower(c);
		if (low == "server") return Config::SERVER;
		if (low == "http") return Config::HTTP;
		return Config::UNKNOWN;
	};
	static Fn<String(std::vector<String>)>	concat = [](std::vector<String> v) noexcept -> String {
		String	out;
		for (std::string s : v) out.append(s);
		return out;
	};
	static Fn<Fn<String(String)>(String)>	prependString = [](String prefix) noexcept -> Fn<String(String)> {
		return [prefix](String postfix) { return prefix + postfix; };
	};

	const Parser<String>			token = some(tchar);
	const Parser<String>			ows = many(parsePredicate(isspace));
	const Parser<String>			rws = some(parsePredicate(isspace));
	extern const Parser<Config::Type>	type = toType >> token;
	extern const Parser<String>			ident = token;
	extern const Parser<String>			value = token;
	extern const Parser<Pair<String,String>>	keyValue([](const String& s) noexcept -> Maybe<Pair<String,Pair<String,String>>> {
			Maybe<Pair<String,String>>	key = (ows > token < ows)(s);
			if (!key) return std::nullopt;
			Maybe<Pair<String,String>>	values = (parseOpt<String>("", (prependString >> (parseString("=") > ows > value < ows))
					* (concat >> many((prependString >> (parseString(",") < ows)) * (value < ows))))
					< parseChar(';'))(key->first);
			if (!values) return std::nullopt;
			return std::make_pair(values->first, std::make_pair(key->second, values->second));
	});
	extern const Parser<String>			commentLine
		= (ows > parseChar('#')
			> many(parsePredicate([](const char& c) -> bool { return !(c == '\n'); }))
			< parseChar('\n'))
		|	ows > parseString("\n")
		|	many(parseChar(';') | parsePredicate([](const char& c) -> bool { return isspace(c) && c != '\n';})) > parseString("\n");
	extern const Parser<Config>			block([](const String& s) noexcept -> Maybe<Pair<String,Config>> {
			static const Parser<std::variant<Pair<String,String>,Config,String>>	localConfigValue([](const String& s) -> Maybe<Pair<String,std::variant<Pair<String,String>,Config,String>>> {
				Maybe<Pair<String,String>>				comment = commentLine(s);
				if (comment) return comment;
				Maybe<Pair<String,Pair<String,String>>>	keyval = keyValue(s);
				if (keyval) return keyval;
				return block(s);
			});
			Maybe<Pair<String,Config::Type>>	typeRes = parseOpt(Config::UNKNOWN, (ows > type))(s);
			if (!typeRes) return std::nullopt;
			Maybe<Pair<String,String>>	identRes = parseOpt<String>("", (ows > ident))(typeRes->first);
			if (!identRes) return std::nullopt;
			Maybe<Pair<String,char>>	openBrace = (ows > parseChar('{') < ows)(identRes->first);
			if (!openBrace) return std::nullopt;
			Maybe<Pair<String,std::vector<std::variant<Pair<String,String>,Config,String>>>>	values = many(localConfigValue)(openBrace->first);
			if (!values) return std::nullopt;
			Maybe<Pair<String,char>>	closingBrace = (ows > parseChar('}') < ows < parseChar(';'))(values->first);
			if (!closingBrace) return std::nullopt;
			Config	out({.type = typeRes->second, .ident = identRes->second, .values = {}, .blocks = {}});
			for (const std::variant<Pair<String,String>,Config,String>&	value : values->second) {
				if (std::holds_alternative<Config>(value)) {
					out.blocks.push_back(std::get<Config>(value));
				} else if (std::holds_alternative<Pair<String,String>>(value)) {
					String	key;
					String	val;
					std::tie(key,val) = std::get<Pair<String,String>>(value);
					if (!out.values.contains(key)) out.values[key] = val;
					else out.values[key] += "," + val;
				}
			}
			return std::make_pair(closingBrace->first, out);
	});
	extern const Parser<std::variant<Pair<String,String>,Config,String>>	configValue([](const String& s) -> Maybe<Pair<String,std::variant<Pair<String,String>,Config,String>>> {
		Maybe<Pair<String,String>>				comment = commentLine(s);
		if (comment) return comment;
		Maybe<Pair<String,Pair<String,String>>>	keyval = keyValue(s);
		if (keyval) return keyval;
		return block(s);
	});
	extern const Parser<Config>			config([](const String& s) noexcept -> Maybe<Pair<String,Config>> {
			Maybe<Pair<String,Config>>	asBlock = block("{" + s + "};");
			if (!asBlock) return std::nullopt;
			asBlock->second.type = Config::TOP_LEVEL;
			asBlock->second.ident = "TopLevelConfiguration";
			return asBlock;
	});
}
