/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Parser.tpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/16 18:59:51 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/16 19:01:06 by mde-beer            ########   odam.nl   */
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
# include <Parser.hpp>

template <typename T>
Parser<T>::Parser(std::function<Maybe<Parser<T>::resultType>(const std::string&)> func) noexcept : f(func) {};
template <typename T>
Parser<T>::Parser(const Parser<T>& p) noexcept : f(p.f) {};

template <typename T>
auto	Parser<T>::runParser(const std::string& s) const noexcept -> Maybe<resultType> {
	return f(s);
}

template <typename T>
auto	Parser<T>::operator()(const std::string& s) const noexcept -> Maybe<resultType> {
	return runParser(s);
}

template <typename T>
template <typename I>
auto	Parser<T>::fmap(std::function<I(T)>	func) noexcept -> Parser<I> {
	Parser<I>	out([this, func](const std::string& s) noexcept -> Maybe<std::pair<std::string,I>> {
			Maybe<Parser<T>::resultType>	inner = this->runParser(s);
			if (inner.has_value()) {
				return std::make_pair(inner.value().first, func(inner.value().second));
			}
			return std::nullopt;
	});
	return out;
}

template <typename T, typename I>
auto	operator>>(std::function<I (T)> lhs, Parser<T> rhs) noexcept -> Parser<I> {
	return rhs.fmap(lhs);
}

template <typename T, typename I>
auto	operator*(Parser<std::function<I(T)>> lhs, Parser<T> rhs) noexcept -> Parser<I> {
	Parser<I>	out([lhs, rhs](const std::string& s) -> Maybe<Pair<std::string,I>> {
			Maybe<Pair<std::string, std::function<I(T)>>>	lres = lhs(s);
			if (!lres.has_value()) return std::nullopt;
			Maybe<Pair<std::string, T>> rres = rhs(lres.value().first);
			if (!rres.has_value()) return std::nullopt;
			return std::make_pair(rres.value().first, lres.value().second(rres.value().second));
	});
	return out;
}

template <typename T>
auto	operator|(Parser<T> lhs, Parser<T> rhs) noexcept -> Parser<T> {
	Parser<T>	out([lhs, rhs](const std::string& s) -> Maybe<Pair<std::string,T>> {
			if (lhs(s).has_value()) return lhs(s);
			return rhs(s);
	});
	return out;
}

template <typename T>
auto	Parse::pure(T x) noexcept -> Parser<T> {
	Parser<T>	out([x](const std::string& s) -> Maybe<Pair<std::string,T>> {
			return std::make_pair(s, x);
	});
}

template <typename T>
auto	Parse::empty() noexcept -> Parser<T> {
	Parser<T>	out([]([[maybe_unused]] const std::string& s) -> Maybe<std::pair<std::string,T>> {
			return std::nullopt;
	});
	return out;
}

template <typename T>
auto	Parse::parseOpt(T x, Parser<T> p) noexcept -> Parser<T> {
	Parser<T>	out([x, p](const std::string& s) -> Maybe<Pair<std::string,T>> const {
			return p(s).value_or(std::make_pair(s, x));
	});
	return out;
}

#endif // PARSER_TPP
