#ifndef DEFINES_HPP
#define DEFINES_HPP

// standart libraries
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <exception>
#include <map>
#include <fstream>
#include <sstream>

// Global variables
extern bool	g_shutdown;

// Custom Outputs
#define RED		"\033[0;31m"
#define GREEN	"\033[0;32m"
#define BLUE	"\033[0;34m"
#define RESET	"\033[0m"
#define CLEAR	"\033c"


// printInfo messages
#define START_MSG					"Webserv is starting..."
#define END_MSG						"Webserv shutdowned"
#define SHUTDOWN_MSG				"Webserv is shutting down..."
#define SET_SERVER_MSG(host, port)	"Server " + host + ":" + port + " setup complete"
#define LAUNCH_MSG					"Launching servers..."

// Parser check input errors
#define ERR_ARG							"Invalid arguments\n\tUsage: ./webserv [config_file]"
#define ERR_FILE						" is a invalid file\n\tFile must have a name and must be .conf"
#define ERR_OPEN						"Couldn't open file "

// Default settings
#define DEFAULT_CONF		"system/default.conf"


#endif