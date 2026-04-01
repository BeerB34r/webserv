/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Server.cpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/10 19:57:48 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/24 16:34:20 by mde-beer            ########   odam.nl   */
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

#include <Server.hpp>

#include <charconv> // std::from_chars()
#include <fstream>

#include <debug.hpp>
#include <webserv.hpp>

// allow for std::string literals etc
using namespace std::literals;

static inline auto	to_int(const std::string& s) noexcept -> std::optional<size_t> {
	size_t	rv{};
	if (std::from_chars(s.data(), s.data() + s.size(), rv).ec == std::errc{}) {
		return rv;
	}
	else return std::nullopt;
}


static auto	ipv4ToLong(const std::string& s) noexcept -> long {
	std::string	addr = (s == "localhost") ? "127.0.0.1" : s;
	long	rv;
	long	firstOctet = *to_int(addr);
	long	secondOctet = *to_int(addr.substr(addr.find('.') + 1));
	long	thirdOctet = *to_int(addr.substr(addr.find('.', addr.find('.') + 1) + 1));
	long	fourthOctet = *to_int(addr.substr(addr.find('.', addr.find('.', addr.find('.') + 1) + 1) + 1));

	rv = (firstOctet << (8 * 3)) + (secondOctet << (8 * 2)) + (thirdOctet << (8 * 1)) + (fourthOctet << (8 * 0));
	return rv;
}

static auto	readFile(const std::string& path) -> Maybe<std::string> {
	std::ifstream	file(path);
	if (!file.is_open()) return std::nullopt;
	std::stringstream	ss;
	ss << file.rdbuf();
	return ss.str();
}

static inline auto	splitOnChar(std::string s, char c) -> std::vector<std::string> {
	std::vector<std::string>	rv;
	s.push_back(c);
	do {
		rv.push_back(s.substr(0, s.find(c)));
		s = s.substr(s.find(c) + 1);
	} while (s.size());
	return rv;
}

static auto	singleServerFromConfig(const Config& c) -> Maybe<Server> {
	Server s;
	s.blocks = c.blocks;
	s.ident = c.ident;
	s.values = c.values;
	if (c.values.at("listen").contains(':')) {
		std::string	prefix = s.values.at("listen").substr(0, s.values.at("listen").find(':'));
		std::string	suffix = s.values.at("listen").substr(s.values.at("listen").find(':') + 1);
		if (prefix == "localhost") prefix = "127.0.0.1";
		s.address = ipv4ToLong(prefix);
		s.port = *to_int(suffix);
	} else { // any address
		s.port = *to_int(c.values.at("listen"));
	}
	if (c.contains(Config::ERROR)) {
		for (Config current : c.getBlocks(Config::ERROR)) for (std::pair<std::string,std::string> p : current.values) {
			if (!to_int(p.first)) continue ;
			int	code = *to_int(p.first);
			Maybe<std::string>	page = readFile(p.second);
			if (!page) {
				WARN("could not read error page " + p.second);
				return std::nullopt;
			}
			if (s.statusPages.contains(code)) {
				s.statusPages.insert_or_assign(code, HTTPMessage(
							s.statusPages.at(code).getStartline(),
							{"content-length:" + std::to_string(page->size())},
							*page
				));
			} else {
				WARN("error code " + std::to_string(code) + " is not supported" );
			}
		}
	}
	if (c.values.contains("cgi")) for (const std::string& val : splitOnChar(c.values.at("cgi"), ',')) s.cgiExts.insert(val.starts_with('.') ? val : "." + val);
	if (c.values.contains("cgidir")) for (const std::string& val : splitOnChar(c.values.at("cgidir"), ',')) s.cgiDirs.insert(val.ends_with('/') ? val : val + "/");
	if (c.values.contains("allowedmethods")) {
		for (const std::string& val : splitOnChar(c.values.at("allowedmethods"), ',')) {
			if (val.empty()) continue ;
			else s.supportedMethods.insert(toHTTPMethod(val));
		}
		for (HTTPMessage::HTTPMethod m : s.supportedMethods) {
			if (!HTTPMessage::supportedRequestMethods.contains(m)) {
				WARN("unsupported method in config file");
				return std::nullopt;
			}
		}
	} else s.supportedMethods = HTTPMessage::supportedRequestMethods;
	if (c.values.contains("datadir")) for (const std::string& val : splitOnChar(c.values.at("datadir"), ',')) s.dataDirs.insert(val);
	if (c.values.contains("maxrequestsize")) s.maxRequestSize = *to_int(c.values.at("maxrequestsize"));
	s.root = c.values.at("root").substr(0, c.values.at("root").find(','));
	s.writeEventHandler = defaultWriteEventHandler;
	s.readEventHandler = defaultReadEventHandler;
	for (const Config& routes : c.getBlocks(Config::ROUTE)) {
		std::set<HTTPMessage::HTTPMethod>	methods = {};
		std::map<std::string,std::string>	netToPath = {};
		for (const std::pair<const std::string, std::string>& p : routes.values) {
			std::string	key = p.first;
			std::string	value = p.second;
			if (key == "allowedmethods") {
				std::string	methodcsv = value + ',';
				do {
					methods.insert(toHTTPMethod(methodcsv.substr(0, methodcsv.find(','))));
					methodcsv = methodcsv.substr(methodcsv.find(',') + 1);
				}	while (methodcsv.size());
				continue ;
			}
			if (!value.ends_with('/')) value.push_back('/');
			if (!key.ends_with('/')) key.push_back('/');
			netToPath.insert(std::make_pair(key, value));
		}
		for (auto& p : netToPath) s.routes.insert(std::make_pair(p.first, std::make_pair(p.second, methods)));
	}
	return s;
}

auto	fromConfig(const Config& c) -> std::vector<Server> {
	std::vector<Server>	rv;

	if (!c.contains(Config::SERVER)) {
		Maybe<Server> s = singleServerFromConfig(c);
		if (s)
			rv.push_back(*s);
	} else for (Config& current : c.getBlocks(Config::SERVER)) {
		Maybe<Server> s = singleServerFromConfig(current);
		if (s)
			rv.push_back(*s);
	}
	return rv;
}
