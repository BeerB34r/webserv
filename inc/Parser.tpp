/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Parser.tpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/16 18:59:51 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/18 19:09:16 by mde-beer            ########   odam.nl   */
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

#ifndef PARSER_TPP
# define PARSER_TPP

#include <Parser.hpp>
#include <utility>

template <typename T>
constexpr Parser<T>::Parser(std::function<Maybe<Parser<T>::resultType>(const std::string&)> func) noexcept : f(func) {};
template <typename T>
constexpr Parser<T>::Parser(const Parser<T>& p) noexcept : f(p.f) {};

template <typename T>
constexpr auto	Parser<T>::runParser(const std::string& s) const noexcept -> Maybe<resultType> {
	return f(s);
}

template <typename T>
constexpr auto	Parser<T>::operator()(const std::string& s) const noexcept -> Maybe<resultType> {
	return runParser(s);
}

template <typename T>
template <typename I>
constexpr auto	Parser<T>::fmap(std::function<I(T)>	func) noexcept -> Parser<I> {
	return Parser<I>([*this, func](const std::string& s) constexpr noexcept -> Maybe<std::pair<std::string,I>> {
			Maybe<Parser<T>::resultType>	inner = (*this).runParser(s);
			if (!inner) return std::nullopt;
			return std::make_pair(inner->first, func(inner->second));
	});
}

template <typename T, typename I>
constexpr auto	operator>>(std::function<I (T)> lhs, Parser<T> rhs) noexcept -> Parser<I> {
	return rhs.fmap(lhs);
}

template <typename T, typename I>
constexpr auto	operator*(Parser<std::function<I(T)>> lhs, Parser<T> rhs) noexcept -> Parser<I> {
	return Parser<I>([lhs, rhs](const std::string& s) constexpr noexcept -> Maybe<Pair<std::string,I>> {
			Maybe<Pair<std::string, std::function<I(T)>>>	lres = lhs(s);
			if (!lres) return std::nullopt;
			Maybe<Pair<std::string, T>> rres = rhs(lres->first);
			if (!rres) return std::nullopt;
			return std::make_pair(rres->first, lres->second(rres->second));
	});
}

template <typename T>
constexpr auto	operator|(Parser<T> lhs, Parser<T> rhs) noexcept -> Parser<T> {
	return Parser<T>([lhs, rhs](const std::string& s) constexpr noexcept -> Maybe<Pair<std::string,T>> {
			Maybe<Pair<std::string,T>>	lres = lhs(s);
			return lres ? lres : rhs(s);
	});
}

template <typename T, typename I>
constexpr auto	operator>(Parser<T> lhs, Parser<I> rhs) noexcept -> Parser<I> {
	return Parser<I>([lhs, rhs](const std::string& s) constexpr noexcept -> Maybe<Pair<std::string,I>> {
			Maybe<Pair<std::string,T>>	lres = lhs(s);
			if (!lres) return std::nullopt;
			return rhs(lres->first);
	});
};

template <typename T, typename I>
constexpr auto	operator<(Parser<T> lhs, Parser<I> rhs) noexcept -> Parser<T> {
	return Parser<T>([lhs, rhs](const std::string& s) constexpr noexcept -> Maybe<Pair<std::string,T>> {
			Maybe<Pair<std::string,T>>	lres = lhs(s);
			if (!lres) return std::nullopt;
			Maybe<Pair<std::string,I>>	rres = rhs(lres->first);
			if (!rres) return std::nullopt;
			return std::make_pair(rres->first, lres->second);
	});
};

template <typename T>
constexpr auto	Parse::pure(T x) noexcept -> Parser<T> {
	return Parser<T>([x](const std::string& s) constexpr noexcept -> Maybe<Pair<std::string,T>> {
			return std::make_pair(s, x);
	});
}

template <typename T>
constexpr auto	Parse::empty() noexcept -> Parser<T> {
	return Parser<T>([]([[maybe_unused]] const std::string& s) constexpr noexcept -> Maybe<std::pair<std::string,T>> {
			return std::nullopt;
	});
}

template <typename T>
constexpr auto	Parse::parseOpt(T x, Parser<T> p) noexcept -> Parser<T> {
	return Parser<T>([x, p](const std::string& s) constexpr noexcept -> Maybe<Pair<std::string,T>> const {
			return p(s).value_or(std::make_pair(s, x));
	});
}

template <typename T>
auto	Parse::many(const Parser<T> p) noexcept -> Parser<std::vector<T>> {
	return Parser<std::vector<T>>([p](const std::string& s)	constexpr noexcept -> Maybe<Pair<std::string,std::vector<T>>> {
			std::string	remainder(s);
			std::vector<T>	result;
			for (Maybe<Pair<std::string,T>> current = p(s); current.has_value(); current = p(remainder)) {
				result.push_back((current.value().second));
				remainder = current.value().first;
			}
			return std::make_pair(remainder, result);
	});
}

template <typename T>
auto	Parse::some(const Parser<T> p) noexcept -> Parser<std::vector<T>> {
	return Parser<std::vector<T>>([p](const std::string& s)	constexpr noexcept -> Maybe<Pair<std::string,std::vector<T>>> {
			Maybe<Pair<std::string,std::vector<T>>>	result = many(p)(s);
			if (!result.has_value()) return std::nullopt;
			if (!result.value().second.size()) return std::nullopt;
			return result;
	});
}
#endif // PARSER_TPP
