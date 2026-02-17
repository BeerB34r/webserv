/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Parser.hpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/16 18:19:11 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/16 19:02:09 by mde-beer            ########   odam.nl   */
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

#ifndef PARSER_HPP
# define PARSER_HPP

#include <functional>
#include <optional>
#include <string>
#include <utility>

template <typename T>
using Maybe = std::optional<T>;

template <typename T, typename I>
using Pair = std::pair<T,I>;

template <typename T>
class Parser {
public:
	using resultType = Pair<std::string,T>;

	Parser() = delete;
	Parser(std::function<Maybe<resultType>(const std::string&)>) noexcept;
	Parser(const Parser&) noexcept;
	Parser&	operator =(const Parser&) noexcept = default;
	virtual	~Parser() noexcept = default;

	auto	runParser(const std::string&) const noexcept -> Maybe<resultType>;
	auto	operator()(const std::string&) const noexcept -> Maybe<resultType>;

	//	Parser is now a functor
	template <typename I>
	auto	fmap(std::function<I(T)>) noexcept -> Parser<I>;

private:
	const std::function<Maybe<resultType>(const std::string&)>	f;
};

// synonym for fmap
template <typename T, typename I>
auto	operator>>(std::function<I(T)>, Parser<T>) noexcept -> Parser<I>;

// sequential application (half of proving Parser is an Applicative)
template <typename T, typename I>
auto	operator*(Parser<std::function<I(T)>>, Parser<T>) noexcept -> Parser<I>;

// alternative application (half of proving Parser is an Alternative)
template <typename T>
auto	operator|(Parser<T>, Parser<T>) noexcept -> Parser<T>;

namespace Parse {

	template<typename T>
	auto	pure(T x) noexcept -> Parser<T>;

	template<typename T>
	auto	empty() noexcept -> Parser<T>;

	template <typename T>
	auto	parseOpt(T, Parser<T>) noexcept -> Parser<T>;
	auto	parseAny() noexcept -> Parser<char>;

	auto	parseChar(char) noexcept -> Parser<char>;
	auto	parseAnyOf(const std::string&) noexcept -> Parser<char>;
	auto	parsePredicate(std::function<bool(char)>) noexcept -> Parser<char>;
	auto	parseString(const std::string&) noexcept -> Parser<std::string>;

	auto	many(const Parser<char>) noexcept -> Parser<std::string>;
	auto	many(const Parser<std::string>) noexcept -> Parser<std::string>;
	auto	some(const Parser<char>) noexcept -> Parser<std::string>;
	auto	some(const Parser<std::string>) noexcept -> Parser<std::string>;
}

#include "Parser.tpp" // template implementation

#endif // PARSER_HPP
