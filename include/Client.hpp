/**
 * @file Client.hpp
 * @brief Header file for the Client class.
 *
 * This file contains the declaration of the Client class, which represents
 * a client connection and manages HTTP requests and responses.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "defines.hpp"
#include "Server.hpp"
#include "utils.hpp"

/**
 * @class Client
 * @brief Represents a client connection handling HTTP requests.
 *
 * The Client class is responsible for managing client requests, checking
 * their validity, and interacting with the associated Server instance.
 */
class Client
{
	private:
		Server		_server; ///< The server associated with the client.
		int			_socket; ///< The socket descriptor for the client connection.
		bool		_sentRequest; ///< Indicates whether a request has been sent.
		std::string	_request; ///< The raw request string from the client.
		time_t		_lastRequest; ///< Timestamp of the last request.
		std::string	_resourcePath; ///< Path to the resource (e.g., ../site/page).
		std::string	_method; ///< The HTTP method (e.g., GET, POST, DELETE).
		std::string	_requestPayload; ///< Payload of the request (body for POST)
		std::map<std::string, std::string>	_headers; ///< HTTP headers of the request.
		
	/** bool		validateHeaders(const std::string& method, const std::string& path, const std::string& version)
		*/		
		void		handleGetRequest(const std::string& path); ///< Handles GET requests.
		void		handlePostRequest(const std::string& path); ///< Handles POST requests.
		void		handleDeleteRequest(const std::string& path); ///< Handles DELETE requests.
		
		void		sendResponse(int statusCode, const std::string& statusMessage, const std::string& body); ///< Sends HTTP response
		void		sendRedirectResponse(int statusCode, const std::string &location); ///< Sends HTTP redirect response
		void		sendErrorResponse(int statusCode, const std::string& statusMessage); ///< Sends HTTP error response

		bool		isCgiPath(const std::string& filePath, const location_t& locationConfig) const; ///< Checks if a file is a CGI script
		bool		fileExists(const std::string& path); ///< used for DELETE requests
		void		uploadFile(const std::string &path); ///< Handles file uploads
		void		executeCgi(const std::string& cgiPath, const std::string& filePath); ///< Executes CGI scripts
		
		bool		isUrlValid(const std::string &url) const; ///< Checks if a URL chars are valid
		bool		_checkRequest();
		bool		_checkFirstLine(std::stringstream &ss);
		bool 		_checkAndGetHeaders(std::stringstream &ss);
		bool		_checkAndGetPayload(std::stringstream &ss);
		int			_checkLocation(std::string &root, std::string &resource, size_t loopCount);
		long		_parseContentLength(const std::string &contentLengthStr) const;
		Client();

	public:
		Client(Server server, int socket);
		~Client();

		void		appendRequest(char const *buffer, size_t size);
		bool		isTimeout() const;
		bool		clientIsReadyToReceive() const;
		// void		sendResponseToClient();
		void		handleClientRequest(); //Janneta's function

		//getters and setters
		const std::string& 	getRequest() const;
		const Server& 		getServer() const;
		void 				changeServer(Server server);
};

#endif


//code lost and recovered when switching to dev branch?
// class Client
// {
// 	private:
// 		Server		_server; ///< The server associated with the client.
// 		int			_socket; ///< The socket descriptor for the client connection.
// 		bool		_sentRequest; ///< Indicates whether a request has been sent.
// 		std::string	_request; ///< The raw request string from the client.
// 		time_t		_lastRequest; ///< Timestamp of the last request.
// 		std::string	_resourcePath; ///< Path to the resource (e.g., ../site/page).
// 		std::string	_method; ///< The HTTP method (e.g., GET, POST, DELETE).
// 		std::string _httpVersion; ///< HTTP version of the request (e.g., HTTP/1.1).
// 		std::string	_requestPayload; ///< Payload of the request (used in POST).
// 		std::map<std::string, std::string>	_headers; ///< HTTP headers of the request.

// 		void		_checkRequest(); ///< Validates the entire HTTP request.
// 		void		_checkFirstLine(std::stringstream &ss); ///< Parses the request line (method, path, version).
// 		void 		_checkAndGetHeaders(std::stringstream &ss); ///< Extracts headers from the request.
// 		void		_checkAndGetPayload(std::stringstream &ss); ///< Extracts the payload for POST requests.
// 		void		_checkLocation(std::string &root, std::string &resource, size_t loopCount); ///< Validates resource location.
// 		bool		fileExists(const std::string& path) const; ///< Checks if a file exists.
// 		void		stringTrim(std::string& str, const std::string& whitespace = " "); ///< Trims whitespace from strings.

// 		void		sendErrorResponse(int statusCode, const std::string& statusMessage); ///< Sends error response to the client.
// 		void		sendResponse(int statusCode, const std::string& statusMessage, const std::string& body); ///< Sends response to the client.

// 		void		handleGetRequest(const std::string& path); ///< Processes GET requests.
// 		void		handlePostRequest(const std::string& path); ///< Processes POST requests.
// 		void		handleDeleteRequest(const std::string& path); ///< Processes DELETE requests.

// 		Client();

// 	public:
// 		Client(Server server, int socket);
// 		~Client();

// 		void		appendRequest(char const *buffer, size_t size); ///< Appends data to the request buffer.
// 		bool		isTimeout() const; ///< Checks if the connection has timed out.
// 		bool		clientIsReadyToReceive() const; ///< Checks if the client is ready to receive data.
// 		void		handleClientRequest(); ///< Main function to process the client request.

// 		// Getters and setters
// 		const std::string& 	getRequest() const; ///< Returns the raw request string.
// 		const Server& 		getServer() const; ///< Returns the associated server.
// 		void 				changeServer(Server server); ///< Updates the associated server.
// };

// #endif



// #ifndef CLIENT_HPP
// #define CLIENT_HPP

// #include "defines.hpp"
// #include "Server.hpp"
// #include <map>
// #include <string>
// #include <ctime>
// #include <sstream>
// #include <fstream>
// #include <iostream>
// #include <sys/stat.h>
// #include <unistd.h>

// class Client
// {
// private:
//     Server _server;                ///< Associated server instance
//     int _socket;                   ///< Client's socket descriptor
//     bool _sentRequest;             ///< Flag to indicate if a request was sent
//     std::string _request;          ///< Raw HTTP request content
//     time_t _lastRequest;           ///< Last request timestamp
//     std::string _resourcePath;     ///< Path to the requested resource
//     std::string _method;           ///< HTTP method (GET, POST, DELETE)
//     std::string _httpVersion;      ///< HTTP version (e.g., HTTP/1.1)
//     std::string _requestPayload;   ///< Data payload for POST requests
//     std::map<std::string, std::string> _headers; ///< Parsed HTTP headers

//     // Private methods
//     bool validateHeaders();                                      ///< Validate HTTP headers
//     bool fileExists(const std::string& path) const;              ///< Check if a file exists
//     void sendErrorResponse(int statusCode, const std::string& statusMessage); ///< Send HTTP error response
//     void sendResponse(int statusCode, const std::string& statusMessage, const std::string& body); ///< Send HTTP response
//     void handleGetRequest(const std::string& path);              ///< Process GET requests
//     void handlePostRequest(const std::string& path);             ///< Process POST requests
//     void handleDeleteRequest(const std::string& path);           ///< Process DELETE requests
//     void processFileUpload();                                    ///< Handle file uploads
//     void executeCgi(const std::string& cgiPath);                 ///< Execute CGI scripts
//     void stringTrim(std::string& str, const std::string& whitespace = " "); ///< Trim whitespace from strings

// public:
//     // Constructor and Destructor
//     Client(Server server, int socket);
//     ~Client();

//     // Public methods
//     void handleClientRequest(); ///< Parse and process client HTTP requests
//     const std::string& getRequest() const; ///< Get the raw HTTP request
// };

// #endif // CLIENT_HPP
