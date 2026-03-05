/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   cmdline.cpp                                             :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/03/05 20:08:33 by mde-beer            #+#    #+#           */
/*   Updated: 2026/03/05 20:09:30 by mde-beer            ########   odam.nl   */
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
#include <global.hpp>
#include <cmdline.hpp>
#include <getopt.h>

auto	runTests(void) -> void;

static inline auto	usage(const char* bin) -> void {
	std::cout << CYAN BOLD "[USAGE]" RESET " " << bin << " [options] <config file>\n";
	std::cout << CYAN BOLD "[OPTS]" RESET ":\n";
	std::cout << "\t-h | --help | --usage\n";
	std::cout << "\t\tshow this message\n";
	std::cout << "\t-v | --verbose\n";
	std::cout << "\t\tturn on verbose terminal output\n";
	std::cout << "\t-l | --log\n";
	std::cout << "\t\tturn on logging (default logfile: \"" << bin << ".log\")\n";
	std::cout << "\t--logfile <logfile>\n";
	std::cout << "\t\tspecify file for logging. implies '-l'\n";
	std::cout << "\t-t | --tests\n";
	std::cout << "\t\trun test suite\n";
	// -f for fun is intentionally hidden :3
}

// returns 0 on success, 1 on happy exit, and 2 on unhappy exit
auto	cmdlineArgs(int& ac, char**& av) -> int {
	const char*	bin = av[0];
	using namespace std::literals;
	if (ac == 1) {
		usage(av[0]);
		return (1);
	}
	global::logfile = av[0] + ".log"s;
	int				opt = 0;
	int				opt_index = 0;
	struct option	long_opts[] = {
		{ "help", no_argument, nullptr,  'h' },
		{ "usage", no_argument, nullptr,  'h' },
		{ "verbose", no_argument, nullptr, 'v'},
		{ "log", no_argument, nullptr, 'l'},
		{ "logfile", required_argument, nullptr, 'o'},
		{ "fun", no_argument, nullptr, 'f'},
		{ "tests", no_argument, nullptr, 't'},
		{} // trailing zero
	};
	while ((opt = getopt_long(ac, av, "hvlft", long_opts, &opt_index)) != -1) {
		switch (opt) {
			case 'h': {
				usage(av[0]);
				return (1);
			}
			case 'v': {
				global::verbose = true;
				break ;
			}
			case 'l': {
				global::log = true;
				break ;
			}
			case 'o': {
				global::log = true;
				global::logfile = optarg;
				break ;
			}
			case 'f': {
				global::fun_allowed = true;
				break ;
			}
			case 't': {
				runTests();
				return (1);
			}
			default: {
				return (2);
			}
		}
	}
	av = &av[optind];
	ac -= optind;
	if (ac != 1) {
		usage(bin);
		return (2);
	}
	return (0);
}
