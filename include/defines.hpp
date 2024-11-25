#ifndef DEFINES_HPP
#define DEFINES_HPP

// standart libraries
#include <iostream>
#include <csignal>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <exception>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
//#include <regex>
#include <fcntl.h>
#include <stdexcept>
#include <cctype>
#include <unistd.h>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <limits>

// Global variables
extern bool	g_shutdown;

//Defaut settings

#define POLL_TIME_OUT		200		// 200ms
#define MAX_PENDING			10		// Maximum number of pending connections
#define BUFFER_SIZE			2048	// 2KB
#define SENT_TIMEOUT		60		// 60s

// Charset 	
#define REQUEST_END			"\r\n\r\n"
#define CURSOR_NEWLINE		"\r\n"
#define REQUEST_HOST		"Host: "

// Custom Outputs
#define RED		"\033[0;31m"
#define GREEN	"\033[0;32m"
#define BLUE	"\033[0;34m"
#define RESET	"\033[0m"
#define CLEAR	"\033c"

// printInfo messages
#define START_MSG						"Webserv is starting..."
#define END_MSG							"Webserv shutdowned"
#define SHUTDOWN_MSG					"Webserv is shutting down..."
#define SET_SERVER_MSG(host, port)		"Server " + host + ":" + port + " setup complete"
#define LAUNCH_MSG						"Launching servers..."
#define EMPTY_MSG						""
#define POLLERR_MSG						"Connection closed. Error: POLLERR"
#define POLLHUP_MSG						"Connection closed. Error: POLLHUP"
#define POLLNVAL_MSG					"Connection closed. Error: POLLNVAL"
#define CLOSE_MSG						"Connection closed"
#define TIMEOUT_MSG						"Connection closed. Timeout"

// Parser check input errors
#define ERR_ARG							"Invalid arguments\n\tUsage: ./webserv [config_file]"
#define ERR_FILE_CONF(confFile)			"'"+ confFile + "' is a invalid file\n\tFile must have a name and must be .conf"
#define ERR_OPEN						"Couldn't open file "
#define ERR_NO_SERVER_CONFIG			"There is no bloc server in the configuration file "

//Parser token error
#define ERR_INVALID_KEY(line)						"Invalid keyword on line: '" + line + "'"
#define ERR_SEMICOLON(line)							"Missing ';' at line '" + line +"'"
#define ERR_CLOSING_BRACKETS						"Missing closing bracket"
#define ERR_OPENING_BRACKET							"Missing opening bracket"
#define ERR_SERV_FORBIDDEN_DIRECTIVE(directive)		"Directive '" + directive + "' is not allowed in Server block"
#define ERR_LOC_FORBIDDEN_DIRECTIVE(directive)		"Directive '" + directive + "' is not allowed in Location block"

//Bloc Server error
#define ERR_PORT_INPUT(port)					"'"+ port + "' is not a valid port number. Port must be a number between 0 and 65535"
#define ERR_HOST_INPUT(host)					"'"+ host + "' is not a valid host name or ip adresse."
#define ERR_DIRECTORY(path)						"'" + path + "' is not a valid directory"
#define ERR_NOT_FILE_NOT_DIR(path)				"'" + path + "' is neither a regular file nor a directory."
#define ERR_FILE_INFO(path)						"Error retrieving file info for: '" + path + "'"
#define ERR_OPEN_DIR(path)						"Unable to open directory: '" + path + "'"
#define ERR_PATH(path)							"'" + path + "' is not a valid path"
#define ERR_WRITE_PERM(path)					"Write access denied for path: '" + path + "'"
#define ERR_SERV_DIRECTIVE_MISSING(directive)	"Missing Directive '" + directive + "' in Server block"
#define ERR_LOC_DIRECTIVE_MISSING(directive)	"Missing Directive '" + directive + "' in Location block"
#define ERR_FILE(file)							"Couldn't open file '" + file + "' ."
#define ERR_MAX_SIZE_INPUT(size)				"'" + size + "' is not a valid size. Size must be a number positive or a number followed by a sufix (b - B, k - K, m - M, g - G)"
#define ERR_MAX_SIZE_RANGE(size)				"'" + size + "' is not a valid size. The max value allowed is 10G (10737418240 bytes)"
#define ERR_MAX_SIZE_CONVERSION(size)			"'" + size + "' is not a valid number."
#define ERR_MAX_SIZE_CONVERSION_LONG(size)		"'" + size + "' is a number too large for long."
#define ERR_INVALID_SERVER_NAME(server)			"'"+ server + "' is not a valid server name, only alphanumeric, hyphens, and periods are allowed "

//Bloc Location Error
#define ERR_LOCATION(path)						"Location's path needs to begin with a '/' "
#define ERR_INVALID_METHOD(method, directive)	"The method '" + method + "' in the directive'value :'" + directive + "' ."
#define ERR_AUTOINDEX							"The autoinde value should be 'on' or 'off'"
#define ERR_ERR_CGI_DOT(extension)				"Extension '" + extension + "' must start with a point (.)" 
#define ERR_ERR_CGI_EXT(extension)				"Unsupported CGI extension: '" + extension + "'" 

// Maximum Limits
#define MAX_LOOP_COUNT 10  // Define the maximum allowed loop iterations for location checks
#define MAX_URI_LENGTH 2048 // adjust as needed

//Setup Error
#define ERR_SOCKET(server)						"failed to create network socket for server " + server

// Service setServersAddress errors
#define ERR_SET_SOCKET					"setsockopt() failed: "
#define ERR_GET_ADDR_INFO				"getaddrinfo() failed: "
#define ERR_BIND_SOCKET					"bind() failed: "
#define ERR_LISTEN_SOCKET				"listen() failed: "

// Default settings
#define DEFAULT_CONF					"confFiles/default.conf"

// Service launch errors
#define ERR_POLL_FAIL					"poll() failed"
#define ERR_ACCEPT_SOCKET				"accept() failed"

// Server parameters
#define SERVER		"server"
#define LISTEN		"listen_port"
#define	HOST		"listen_host"
#define INDEX		"index"
#define MAX_SIZE	"client_max_body_size"
#define SERVER_N	"server_name"
#define ERROR_P		"error_page"

// Servers'Location parameters
#define LOCATION	"location"
#define ALLOW_M		"allow_methods"
#define TRY 		"try_file"
#define RETURN		"return"
#define AUTOID		"autoindex"
#define ROOT_LOC	"root"
#define UPLOAD		"upload_to"
#define CGI_P		"cgi_path"
#define CGI_E		"cgi_ext"
#define HAS_CGI		"hasCgi"

#endif
