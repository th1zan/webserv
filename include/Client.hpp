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
