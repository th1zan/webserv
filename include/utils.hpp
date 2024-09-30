#ifndef UTILS_HPP
#define UTILS_HPP

#include "defines.hpp"


/* === Time === */
std::string		getTime();

/* === Logs === */
void	printInfo(std::string const &s, std::string const &color);

/* === Signal === */
void	signalHandler(int signum);

#endif