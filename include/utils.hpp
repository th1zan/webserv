#ifndef UTILS_HPP
#define UTILS_HPP

#include "defines.hpp"

/* === String === */

/* === Time === */
std::string		getTime();

/* === Logs === */
void	printInfo(std::string const &s, std::string const &color);

/* === Signal === */
void	signalHandler(int signum);

#endif