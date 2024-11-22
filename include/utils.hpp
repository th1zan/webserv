/**
 * @file utils.hpp
 * @brief Provides utility functions for various operations such as time retrieval,
 * signal handling, logging, and printing information.
 *
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "defines.hpp"

/* === Time === */
std::string		getTime();

/* === Signal === */
void	signalHandler(int signum);

/* === PrintInfo === */
void printMap(const std::map<std::string, std::string>& mapToPrint);
void printInfo(std::string const &s, std::string const &color);

/*=== Missing c++98 fucntions ===*/
long ft_stoll(const std::string& str);

/* === Janneta's utils functions HTTP parsing === */ 
void stringTrim(std::string &str); 
// bool startsWith(const std::string& str, const std::string& prefix); // not used so far in current version

#endif
