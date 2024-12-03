
/**
 * @file Client.cpp
 * @brief Implementation of the Client class for handling client requests in the web server.
 *
 * This file contains the definitions of the member functions of the Client class, which
 * is responsible for managing client connections, processing requests, and sending responses
 * back to the clients. The class handles different HTTP methods such as GET, POST, and DELETE.
 *
 * 
 */

#include "Client.hpp"

// *** Constructor and destructor 
Client::Client(Server server, int socket) : _server(server), _socket(socket), _sentRequest(false), _lastRequest(std::time(NULL)){}
Client::~Client(){}

// *** Getters and Setters
const std::string& Client::getRequest() const{return (this->_request);}
const	Server& Client::getServer() const{return (this->_server);}
void	Client::changeServer(Server server){this->_server = server;}


/**
 * @brief If the buffer is not empty (there is still bytes read by the recv()), its content is append to the Client's _request variable.
 * 
 * This function is public and called from a Service instance.
 * 
 * @param buffer variable with data to append to the request
 * @param size size of the buffer
 */
void	Client::appendRequest(char const *buffer, size_t size)
{
	this->_lastRequest = std::time(NULL);
	this->_sentRequest = false;
	this->_request.append(buffer, size);
}

/**
 * @brief The function checks if there is no timeout between the time the server receive the request and time to respond
 * (= if the connection and the server are fast enough)
 */
bool	Client::isTimeout() const
{
	return (std::time(NULL) - this->_lastRequest > SENT_TIMEOUT);
}

/**
 * @brief The function checks if the client has finished to send a request, so he's ready to receive.
 * The client is ready if 
 * a) "sentRequest = true" 
 * OR 
 * b) the request ends by the sequence (REQUEST_END) "\r\n\r\n"
 */
bool	Client::clientIsReadyToReceive() const
{
	
	return (this->_sentRequest == true || this->_request.find(REQUEST_END) != std::string::npos);
}

