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

auto	Parse::parseAny() noexcept -> Parser<char> {
	return Parser<char>([](const std::string& s)	constexpr noexcept -> Maybe<Parser<char>::resultType> {
			if (s.empty()) return std::nullopt;
			return Parser<char>::resultType(s.data() + 1, s.front());
	});
}

auto	Parse::parseChar(char c) noexcept -> Parser<char> {
	return Parser<char>([c](const std::string& s)	constexpr noexcept -> Maybe<Parser<char>::resultType> {
			if (s.empty()) return std::nullopt;
			else if (s.front() == c)
				return Parser<char>::resultType(s.data() + 1, s.front());
			else
				return std::nullopt;
	});
}

auto	Parse::parseAnyOf(const std::string& set) noexcept -> Parser<char> {
	return Parser<char>([set](const std::string& s)	noexcept -> Maybe<Pair<std::string,char>> {
			Maybe<Pair<std::string,char>>	inner = parseAny()(s);
			if (!inner.has_value()) return std::nullopt;
			if (!set.contains(inner.value().second)) return std::nullopt;
			return inner;
	});
}

auto	Parse::parsePredicate(std::function<bool(char)> f) noexcept -> Parser<char> {
	return Parser<char>([f](const std::string& s)	noexcept -> Maybe<Pair<std::string,char>> {
			Maybe<Pair<std::string,char>>	inner = parseAny()(s);
			if (!inner.has_value()) return std::nullopt;
			if (!f(inner.value().second)) return std::nullopt;
			return inner;
	});
}

// canonically implemented by creating a traversible of parsers and then
// applying them in sequence, however since parseString is pretty simple, and we
// dont need the extensibility, this will suffice
auto	Parse::parseString(const std::string& string) noexcept -> Parser<std::string> {
	return Parser<std::string>([string](const std::string& s)	constexpr noexcept -> Maybe<Pair<std::string,std::string>> {
			if (!s.starts_with(string)) return std::nullopt; 
			return std::make_pair(s.data() + string.size(), string);
	});
}

auto	Parse::many(const Parser<char> p) noexcept -> Parser<std::string> {
	return Parser<std::string>([p](const std::string& s)	constexpr noexcept -> Maybe<Pair<std::string,std::string>> {
			std::string	remainder(s);
			std::string	result;
			for (Maybe<Pair<std::string,char>> current = p(s); current.has_value(); current = p(remainder)) {
				result += (current.value().second);
				remainder = current.value().first;
			}
			return std::make_pair(remainder, result);
	});
}

auto	Parse::some(const Parser<char> p) noexcept -> Parser<std::string> {
	return Parser<std::string>([p](const std::string& s)	noexcept -> Maybe<Pair<std::string,std::string>> {
			Maybe<Pair<std::string,std::string>>	result = many(p)(s);
			if (!result.has_value()) return std::nullopt;
			if (!result.value().second.size()) return std::nullopt;
			return result;
	});
}
