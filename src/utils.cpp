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

long ft_stoll(const std::string& str) {
    if (str.empty()) return 0;

    long result = 0;
    int sign = 1;
    size_t i = 0;

    // Check for leading whitespace
    while (i < str.size() && std::isspace(str[i])) {
        ++i;
    }

    // Check for sign
    if (i < str.size() && (str[i] == '+' || str[i] == '-')) {
        sign = (str[i] == '-') ? -1 : 1;
        ++i;
    }

    // Process each digit
    while (i < str.size() && std::isdigit(str[i])) {
        int digit = str[i] - '0';

        // Check for overflow
        if (result > (std::numeric_limits<long>::max() - digit) / 10) {
            return (sign == 1) ? std::numeric_limits<long>::max() : std::numeric_limits<long>::min();
        }

        result = result * 10 + digit;
        ++i;
    }

    return result * sign;
}
