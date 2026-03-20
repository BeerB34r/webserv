/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   HTTPMessage.hpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/11 14:30:55 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/19 17:02:36 by mde-beer            ########   odam.nl   */
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

#ifndef HTTPMESSAGE_HPP
# define HTTPMESSAGE_HPP

#include <map>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

template <typename T>
using Maybe = std::optional<T>;

class	HTTPMessage {
public:
	// subtypes
	enum HTTPMethod {
		GET,
		HEAD,
		POST,
		PUT,
		DELETE,
		CONNECT,
		OPTIONS,
		TRACE,
		PATCH,
		MOVE,
		COPY,
		LINK,
		UNLINK,
		WRAPPED,
		UNKNOWN,
		UNSUPPORTED
	};
	struct RequestData {
		// from startline
		enum HTTPMethod	method;
		std::string	requestTarget;
		std::string	httpVersion;
	};
	struct ResponseData {
		// from startline
		std::string	httpVersion;
		int			statusCode;
		std::string	reasonPhrase;
	};

	HTTPMessage() = delete;
	HTTPMessage(const std::string& startline, const std::vector<std::string>& fieldlines = {}, const std::string& body = "") noexcept;
	HTTPMessage(const HTTPMessage&) noexcept = default;
	HTTPMessage&	operator=(const HTTPMessage&) noexcept = default;
	virtual	~HTTPMessage() noexcept = default;

	auto	getStartline(void) const noexcept -> const std::string&;
	auto	getFieldlines(void) const noexcept -> const std::vector<std::string>&;
	auto	getFields(void) const noexcept -> const std::map<std::string,std::string>&;
	auto	getBody(void) const noexcept -> const std::string&;
	auto	isRequest(void) const noexcept -> bool;
	auto	getData(void) const noexcept -> const std::variant<RequestData,ResponseData>&;
	auto	prettyPrint(void) const noexcept -> const std::string;
	static const std::set<HTTPMethod>	supportedRequestMethods;
private:
	std::variant<RequestData,ResponseData>	data;
	std::map<std::string,std::string>		fields;
	// plumbing
	std::string								startline;
	std::vector<std::string>				fieldlines;
	std::string								body;
protected:
};

auto	readHTTPmessage(const std::string&) noexcept -> Maybe<HTTPMessage>;
auto	readHTTPrequest(const std::string&) noexcept -> Maybe<HTTPMessage>;
auto	readHTTPresponse(const std::string&) noexcept -> Maybe<HTTPMessage>;
auto	toHTTPMethod(const std::string&) noexcept -> HTTPMessage::HTTPMethod;
auto	toString(const HTTPMessage::HTTPMethod& m) noexcept -> std::string;
auto	operator<<(std::ostream&, const HTTPMessage&) noexcept -> std::ostream&;

#endif // HTTPMESSAGE_HPP
