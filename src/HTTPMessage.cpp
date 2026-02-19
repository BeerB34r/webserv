/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   HTTPMessage.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/19 16:07:22 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/19 17:20:16 by mde-beer            ########   odam.nl   */
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

#include <sstream>
#include <HTTPMessage.hpp>
#include <HTTPparsing.hpp>
#include <Parser.hpp>

using String = std::string;

HTTPMessage::HTTPMessage(const String& startline, const std::vector<String>& fieldlines, const String& body) noexcept
: startline(startline), fieldlines(fieldlines), body(body) {}

auto	HTTPMessage::getStartline(void) const noexcept -> const String& { return this->startline; }
auto	HTTPMessage::getFieldlines(void) const noexcept -> const std::vector<String>& { return this->fieldlines; }
auto	HTTPMessage::getBody(void) const noexcept -> const String& { return this->body; }

auto	readHTTPmessage(const String& s) noexcept -> Maybe<HTTPMessage> {
	Maybe<Parser<String>::resultType>	startlineParseRes = (HTTPparsing::startLine < HTTPparsing::crlf)(s);
	if (!startlineParseRes) return std::nullopt;
	Maybe<Parser<std::vector<String>>::resultType>	fieldlinesParseRes = (HTTPparsing::fieldLines < HTTPparsing::crlf)(startlineParseRes->first);
	if (!fieldlinesParseRes) return std::nullopt;
	Maybe<Parser<String>::resultType>	bodyParseRes = HTTPparsing::messageBody(fieldlinesParseRes->first);
	if (!bodyParseRes) return std::nullopt;
	return HTTPMessage(startlineParseRes->second, fieldlinesParseRes->second, bodyParseRes->second);
}

auto	HTTPMessage::prettyPrint(void) const noexcept -> const String {
	std::stringstream	ss;
	ss << "startline: \"" << this->getStartline() << "\"\n";
	ss << "fieldlines: [\n";
	for (const String& s : this->getFieldlines()) ss << "\t\"" << s << "\"\n";
	ss << "]\n";
	ss << "messagebody:\n\"" << this->getBody() << "\"\n";
	return ss.str();
}

//	for direct insertion into client ostreams
auto	operator<<(std::ostream& os, const HTTPMessage& message) noexcept -> std::ostream& {
	os << message.getStartline() << "\r\n";
	for (const String& s : message.getFieldlines()) os << s << "\r\n";
	os << "\r\n";
	os << message.getBody();
	return os;
}
