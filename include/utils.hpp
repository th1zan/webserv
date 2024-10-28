#ifndef UTILS_HPP
#define UTILS_HPP

#include "defines.hpp"

/* === String === */

/* === Time === */
std::string		getTime();

/* === Logs === */

/* === Signal === */
void	signalHandler(int signum);

/* === PrintInfo === */

void printMap(const std::map<std::string, std::string>& mapToPrint);
void printInfo(std::string const &s, std::string const &color);



#endif
