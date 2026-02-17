/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   parse.cpp                                               :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/16 20:12:28 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/16 20:21:01 by mde-beer            ########   odam.nl   */
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
#include <utility>

auto	Parse::parseAny() noexcept -> Parser<char>{
	Parser<char>	out([](const std::string& s) noexcept -> Maybe<Parser<char>::resultType> {
			if (s.empty()) return std::nullopt;
			return Parser<char>::resultType(s.data() + 1, s.front());
	});

	return out;
}

auto	Parse::parseChar(char c) noexcept -> Parser<char> {
	Parser<char>	out([c](const std::string& s) noexcept -> Maybe<Parser<char>::resultType> {
			if (s.empty()) return std::nullopt;
			else if (s.front() == c)
				return Parser<char>::resultType(s.data() + 1, s.front());
			else
				return std::nullopt;
	});

	return out;
}

auto	Parse::parseAnyOf(const std::string& set) noexcept -> Parser<char> {
	Parser<char>	out([set](const std::string& s) noexcept -> Maybe<Pair<std::string,char>> {
			Maybe<Pair<std::string,char>>	inner = parseAny()(s);
			if (!inner.has_value()) return std::nullopt;
			if (!set.contains(inner.value().second)) return std::nullopt;
			return inner;
	});
	return out;
}

auto	Parse::parsePredicate(std::function<bool(char)> f) noexcept -> Parser<char> {
	Parser<char>	out([f](const std::string& s) noexcept -> Maybe<Pair<std::string,char>> {
			Maybe<Pair<std::string,char>>	inner = parseAny()(s);
			if (!inner.has_value()) return std::nullopt;
			if (!f(inner.value().second)) return std::nullopt;
			return inner;
	});
	return out;
}

// canonically implemented by creating a traversible of parsers and then
// applying them in sequence, however since parseString is pretty simple, and we
// dont need the extensibility, this will suffice
auto	Parse::parseString(const std::string& string) noexcept -> Parser<std::string> {
	Parser<std::string>	out([string](const std::string& s) noexcept -> Maybe<Pair<std::string,std::string>> {
			if (!s.starts_with(string)) return std::nullopt; 
			return std::make_pair(s.data() + string.size(), string);
	});
	return out;
}
