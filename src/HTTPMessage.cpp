/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   HTTPMessage.cpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/19 16:07:22 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/19 18:23:42 by mde-beer            ########   odam.nl   */
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
#include <sstream>
#include <HTTPMessage.hpp>
#include <HTTPparsing.hpp>
#include <Parser.hpp>

using String = std::string;

const std::set<HTTPMessage::HTTPMethod>	HTTPMessage::supportedRequestMethods = {
	HTTPMessage::GET,
	HTTPMessage::POST,
	HTTPMessage::DELETE
};

static inline auto	toRequestMethod(const HTTPMessage::HTTPMethod& method) noexcept -> HTTPMessage::HTTPMethod{
	return HTTPMessage::supportedRequestMethods.contains(method) ? method : HTTPMessage::UNSUPPORTED;
}

static inline auto	populateRequestData(const String& startline) noexcept -> HTTPMessage::RequestData {
	String	method = startline.substr(0, startline.find(' '));
	String	requestTarget = startline.substr(startline.find(' ') + 1, startline.rfind(' ') - (startline.find(' ') + 1));
	String	httpVersion = startline.substr(startline.rfind(' ') + 1);
	return {toRequestMethod(toHTTPMethod(method)), requestTarget, httpVersion};
}

static inline auto	populateFields(const std::vector<String>& fieldlines) -> std::map<String,String> {
	std::map<String,String>	fields;
	for (String s : fieldlines) {
		String	key = s.substr(0, s.find(':'));
		String	value = s.substr(s.find(':') + 1, s.size() - s.find(':'));
		if (fields.contains(key)) {
			fields[key] += "," + value;
		} else fields[key] = value;
	}
	return fields;
}

static inline auto	populateResponseData(const String& startline) -> HTTPMessage::ResponseData {
	String	httpVersion = startline.substr(0, startline.find(' '));
	String	statusCode = startline.substr(startline.find(' ') + 1, 3);
	String	reasonPhrase = startline.substr(startline.find(statusCode) + 4);
	return {httpVersion, std::stoi(statusCode), reasonPhrase};
}

HTTPMessage::HTTPMessage(const String& startline, const std::vector<String>& fieldlines, const String& body) noexcept : startline(startline), fieldlines(fieldlines), body(body) {
	if (HTTPparsing::requestLine(startline)) {
		requestStatus = true;
		data = populateRequestData(startline);
		fields = populateFields(fieldlines);
	} else {
		requestStatus = false;
		data = populateResponseData(startline);
		fields = populateFields(fieldlines);
	}
}

auto	HTTPMessage::getStartline(void) const noexcept -> const String& { return this->startline; }
auto	HTTPMessage::getFieldlines(void) const noexcept -> const std::vector<String>& { return this->fieldlines; }
auto	HTTPMessage::getBody(void) const noexcept -> const String& { return this->body; }
auto	HTTPMessage::isRequest(void) const noexcept -> bool { return this->requestStatus; }

auto	readHTTPmessage(const String& s) noexcept -> Maybe<HTTPMessage> {
	Maybe<Parser<String>::resultType>	startlineParseRes = (HTTPparsing::startLine < HTTPparsing::crlf)(s);
	if (!startlineParseRes) return std::nullopt;
	Maybe<Parser<std::vector<String>>::resultType>	fieldlinesParseRes = (HTTPparsing::fieldLines < HTTPparsing::crlf)(startlineParseRes->first);
	if (!fieldlinesParseRes) return std::nullopt;
	// [TODO]: check for Content-Length field and fail if theres a mismatch
	// NOTE: since this takes strings, that might happen even earlier? will
	// investigate as it becomes relevant -Mats
	Maybe<Parser<String>::resultType>	bodyParseRes = HTTPparsing::messageBody(fieldlinesParseRes->first);
	if (!bodyParseRes) return std::nullopt;
	return HTTPMessage(startlineParseRes->second, fieldlinesParseRes->second, bodyParseRes->second);
} // requestLine == request
  // statusLine == response


// [TODO]: return some actual value on failure so as to make the response accurate
auto	readHTTPrequest(const String& s) noexcept -> Maybe<HTTPMessage> {
	Maybe<Parser<String>::resultType>	requestlineParseRes = (HTTPparsing::requestLine < HTTPparsing::crlf)(s);
	if (!requestlineParseRes) return std::nullopt; // from here on, everything is copied wholesale from readHTTPmessage()
	Maybe<Parser<std::vector<String>>::resultType>	fieldlinesParseRes = (HTTPparsing::fieldLines < HTTPparsing::crlf)(requestlineParseRes->first);
	if (!fieldlinesParseRes) return std::nullopt;
	// [TODO]: check for Content-Length field and fail if theres a mismatch
	// NOTE: since this takes strings, that might happen even earlier? will
	// investigate as it becomes relevant -Mats
	Maybe<Parser<String>::resultType>	bodyParseRes = HTTPparsing::messageBody(fieldlinesParseRes->first);
	if (!bodyParseRes) return std::nullopt;
	return HTTPMessage(requestlineParseRes->second, fieldlinesParseRes->second, bodyParseRes->second);
}

auto	readHTTPresponse(const String& s) noexcept -> Maybe<HTTPMessage> {
	Maybe<Parser<String>::resultType>	statuslineParseRes = (HTTPparsing::statusLine < HTTPparsing::crlf)(s);
	if (!statuslineParseRes) return std::nullopt; // from here on, everything is copied wholesale from readHTTPmessage()
	Maybe<Parser<std::vector<String>>::resultType>	fieldlinesParseRes = (HTTPparsing::fieldLines < HTTPparsing::crlf)(statuslineParseRes->first);
	if (!fieldlinesParseRes) return std::nullopt;
	// [TODO]: check for Content-Length field and fail if theres a mismatch
	// NOTE: since this takes strings, that might happen even earlier? will
	// investigate as it becomes relevant -Mats
	Maybe<Parser<String>::resultType>	bodyParseRes = HTTPparsing::messageBody(fieldlinesParseRes->first);
	if (!bodyParseRes) return std::nullopt;
	return HTTPMessage(statuslineParseRes->second, fieldlinesParseRes->second, bodyParseRes->second);
}

auto	HTTPMessage::prettyPrint(void) const noexcept -> const String {
	std::stringstream	ss;
	if (requestStatus) {
		const RequestData	request = std::get<RequestData>(data);
		ss << request.method << " request\n";
		ss << "request target: \"" << request.requestTarget << "\"\n";
		ss << "version: " << request.httpVersion << "\n";
	} else {
		const ResponseData	response = std::get<ResponseData>(data);
		ss << "status " << response.statusCode << ", \"" << response.reasonPhrase << "\"\n";
		ss << "version: " << response.httpVersion << "\n";
	}
	ss << "fieldlines: [\n";
	for (const std::pair<String,String> s : fields) {
		ss << "\t" << s.first << ": " << s.second << "\n";
	}
	ss << "]\n";
	ss << "messagebody (" << body.size() << "B):\n\"" << body << "\"\n";
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


#define _methodPair(method) {#method, HTTPMessage::method}
auto	toHTTPMethod(const String& s) noexcept -> HTTPMessage::HTTPMethod {
	const static int	knownMethodCount = 14;
	const static struct MethodPair { String s; HTTPMessage::HTTPMethod method; }	methodPairs[knownMethodCount] = {
		_methodPair(GET),
		_methodPair(HEAD),
		_methodPair(POST),
		_methodPair(PUT),
		_methodPair(DELETE),
		_methodPair(CONNECT),
		_methodPair(OPTIONS),
		_methodPair(TRACE),
		_methodPair(PATCH),
		_methodPair(MOVE),
		_methodPair(COPY),
		_methodPair(LINK),
		_methodPair(UNLINK),
		_methodPair(WRAPPED)
	};

	for (struct MethodPair pair : methodPairs) {
		if (pair.s == s) return pair.method;
	}
	return HTTPMessage::UNKNOWN;
}
