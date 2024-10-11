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
#include <vector>


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

//Parser token error
#define ERR_INVALID_KEY(nbline, line)			"Invalid keyword on line " + nbLine+ " : '" + line "'"
#define ERR_SEMICOLON(nbline, line)				"Missing ';' at line " + nbLine+ " : '" + line "'"
#define ERR_CLOSING_BRACKETS					"Missing closing bracket"
#define ERR_OPENING_BRACKET						"Missing opening bracket"

// Default settings
#define DEFAULT_CONF		"system/default.conf"



// Server parameters
#define ALLOW_M		"allow_methods"
#define AUTOID		"autoindex"
#define CGI_E		"cgi_ext"
#define CGI_P		"cgi_path"
#define MAX_SIZE	"client_max_body_size"
#define ERROR_P		"error_page"
#define	HOST		"host"
#define INDEX		"index"
#define LISTEN		"listen"
#define LOCATION	"location"
#define RETURN		"return"
#define ROOT		"root"
#define SERVER		"server"
#define SERVER_N	"server_name"
#define TRY 		"try_file"
#define UPLOAD		"upload_to"


#endif