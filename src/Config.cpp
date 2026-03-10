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

#include <Config.hpp>
#include <charconv>
#include <debug.hpp>
#include <fstream>
#include <set>
#include <sstream>

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
	if (!to_int(c.values.at("listen"))) {
		WARN("\"" + c.values.at("listen") + "\" does not evaluate to an integer");
		rv = true;
	}
	if (to_int(c.values.at("listen")) < 1) {
		WARN("Ports must be non-zero positive integers");
		rv = true;
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
			std::set<int>	ports;
			for (const Config& s : servers) {
				INFO("checking server config \"" + (s.ident.empty() ? "N/A" : s.ident) + "\"...");
				if (checkSingleServer(s)) return true;
				int	port = to_int(s.values.at("listen")).value();
				if (ports.contains(port)) {
					WARN("port '" + s.values.at("listen") + "' reserved by different server");
					return true;
				}
				ports.insert(port);
			}
		}
	}
	return false;
}
