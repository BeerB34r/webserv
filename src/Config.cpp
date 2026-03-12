/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   Config.cpp                                              :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/04 19:53:29 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/10 20:10:40 by mde-beer            ########   odam.nl   */
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

#include "HTTPparsing.hpp"
#include <Config.hpp>
#include <charconv>
#include <debug.hpp>
#include <fstream>
#include <set>
#include <sstream>
#include <utility>

auto	Config::empty(void) const noexcept -> bool {
	return (blocks.empty() && values.empty());
}

auto	Config::propogateToBlocks(void) noexcept -> bool {
	for (Config& c : blocks) {
		for (const std::pair<const std::string,std::string>&	p : values) {
			std::string	key;
			std::string	val;
			std::tie(key,val) = p;
			if (!c.values.contains(key)) c.values[key] = val;
			else c.values[key] += "," + val;
		}
		if (c.propogateToBlocks()) return (true);
	}
	return (false);
}

auto	Config::valid(void) const noexcept -> bool {
	return !isInvalid(*this);
}

auto	fromType(Config::Type t) noexcept -> const std::string {
	switch (t) {
		case (Config::HTTP): return "Http";
		case (Config::SERVER): return "Server";
		case (Config::TOP_LEVEL): return "Config file";
		default: return "";
	}
}

auto	Config::getBlocks(Config::Type t) const noexcept -> std::vector<Config> {
	std::vector<Config>	rv;

	for (const Config& c : blocks) {
		if (c.type == SERVER) rv.push_back(c);
		else for (const Config& server : c.getBlocks(t)) {
			rv.push_back(server);
		}
	}
	return rv;
}

auto	readConfigFile(const std::string &fname) noexcept -> std::optional<Config> {
	std::ifstream	fs(fname);
	if (!fs.is_open()) {
		WARN("could not open " + fname);
		return std::nullopt;
	}
	std::ostringstream	oss;
	oss << fs.rdbuf();
	std::string	content = oss.str();
	return ConfigParse::config(content).transform([](auto p){ return p.second; });
}

auto	Config::contains(Config::Type t) const noexcept -> bool {
	for (const Config& c : blocks) {
		if (c.type == t) return true;
		if (c.contains(t)) return true;
	}
	return false;
}

static auto	to_int(const std::string& s) noexcept -> std::optional<int> {
	int	rv{};
	if (std::from_chars(s.data(), s.data() + s.size(), rv).ec == std::errc{}) {
		return rv;
	}
	else return std::nullopt;
}

static auto	checkSingleServer(const Config& c) -> bool {
	bool	rv = false;
	if (!c.values.contains("listen")) {
		WARN("server lacks \"listen\" key");
		rv = true;
	}
	if (!c.values.at("listen").contains(':')) { // just port
		if (to_int(c.values.at("listen")) < 1) {
			WARN("ports must be non-zero positive integers");
			rv = true;
		}
	} else { // ip:port
		Maybe<std::pair<std::string,std::string>>	ip = HTTPparsing::ipv4address(c.values.at("listen"));
		if (!ip && c.values.at("listen").substr(0, c.values.at("listen").find(':')) != "localhost") {
			WARN("listening address is not valid");
			rv = true;
		}
		if (ip) {
			if ((Parse::parseChar(':') > HTTPparsing::port)(ip->second)) {
				WARN("address cannot be provided without associated port");
				rv = true;
			}
		}
	}
	if (!c.values.contains("root")) {
		WARN("server lacks \"root\" key");
		rv = true;
	}
	if (c.contains(Config::SERVER)) {
		WARN("server cannot recurse");
		rv = true;
	}
	return rv;
}

static auto	ipv4ToLong(const std::string& s) noexcept -> long {
	std::string	addr = (s == "localhost") ? "127.0.0.1" : s;
	long	rv;
	long	firstOctet = *to_int(addr);
	long	secondOctet = *to_int(addr.substr(addr.find('.')));
	long	thirdOctet = *to_int(addr.substr(addr.find('.', addr.find('.') + 1)));
	long	fourthOctet = *to_int(addr.substr(addr.find('.', addr.find('.', addr.find('.') + 1) + 1)));

	rv = (firstOctet << (8 * 3)) + (secondOctet << (8 * 2)) + (thirdOctet << (8 * 1)) + (fourthOctet << (8 * 0));
	return rv;
}

auto	isInvalid(const Config& c) noexcept -> bool {
	if (c.empty()) {
		WARN("config is essentially empty");
		return true;
	}
	if (!c.contains(Config::SERVER)) {
		INFO("config contains no \"server\" blocks, assuming entire config describes server");
		return checkSingleServer(c);
	} else {
		std::vector<Config>	servers = c.getBlocks(Config::SERVER);
		if (servers.size() == 1) {
			INFO("config contains only one \"server\" block");
			return checkSingleServer(servers[0]);
		}
		else {
			std::set<int>					ports;
			std::set<std::pair<long,int>>	fullAddresses;
			for (const Config& s : servers) {
				INFO("checking server config \"" + (s.ident.empty() ? "N/A" : s.ident) + "\"...");
				if (checkSingleServer(s)) return true;
				int port;
				if (!s.values.at("listen").contains(':')) {
					port = to_int(s.values.at("listen")).value();
					if (ports.contains(port)) {
						WARN("port '" + s.values.at("listen") + "' reserved by different server");
						return true;
					}
					ports.insert(port);
				} else {
					std::string	prefix = s.values.at("listen").substr(0, s.values.at("listen").find(':'));
					std::string	suffix = s.values.at("listen").substr(s.values.at("listen").find(':') + 1);
					if (prefix == "localhost") prefix = "127.0.0.1";
					long addr = ipv4ToLong(HTTPparsing::ipv4address(prefix)->second);
					port = to_int(suffix).value();
					if (fullAddresses.contains(std::make_pair(addr, port))) {
						WARN("network address '" + s.values.at("listen") + "' reserved by different server");
						return true;
					}
					fullAddresses.insert(std::make_pair(addr, port));
				}
			}
		}
	}
	return false;
}
