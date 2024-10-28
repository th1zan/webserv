#include "utils.hpp"
#include "defines.hpp"
#include "Server.hpp"

bool g_shutdown = false;

/* === Time === */
std::string getTime()
{
	time_t		now = time(0);
	struct tm	tstruct;
	char		buf[9];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%H:%M:%S", &tstruct);
	return (buf);
}

/* === Logs === */
void printInfo(std::string const &s, std::string const &color){
	if (s == START_MSG)
		std::cout << CLEAR;

	if (s == SHUTDOWN_MSG)
		std::cout << std::endl;

	std::cout << color << "[" << getTime() << "]" << RESET << s << std::endl;
}


/* === Signal === */

void signalHandler(int signum){
	if (signum == SIGINT){
		g_shutdown = true;
		printInfo(SHUTDOWN_MSG, GREEN);
	}
}

/* === PrintInfo === */
void printMap(const std::map<std::string, std::string>& mapToPrint){
	for (std::map<std::string, std::string>::const_iterator it = mapToPrint.begin(); it != mapToPrint.end(); ++it) {
		std::cout << "Key: '" << it->first << "', Value: '" << it->second << "'" << std::endl;
	}
}

