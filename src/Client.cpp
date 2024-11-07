
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

// *** Public functions
void	Client::appendRequest(char const *buffer, size_t size)
{
	this->_lastRequest = std::time(NULL);
	this->_sentRequest = false;
	this->_request.append(buffer, size);
}

// *** Getters and Setters
const std::string& Client::getRequest() const{return (this->_request);}
const	Server& Client::getServer() const{return (this->_server);}
void	Client::changeServer(Server server){this->_server = server;}

bool	Client::isTimeout() const
{
	return (std::time(NULL) - this->_lastRequest > SENT_TIMEOUT);
}

bool	Client::clientIsReadyToReceive() const
{
	// The client has finished to send a request, so he's ready to receive.
	// The client is ready if a) "sentRequest = true" OR b) the request ends by the sequence (REQUEST_END) "\r\n\r\n"
	return (this->_sentRequest == true || this->_request.find(REQUEST_END) != std::string::npos);
}

/* --- Jannetta function ---*/

void Client::handleClientRequest()
{
	
	std::istringstream requestStream(this->_request);
	std::string method, path, version;
	requestStream >> method >> path >> version;
	
	// GET request
	if (method == "GET")
	{
		std::string filePath = this->_server.getRoot() + path;
		//DEBUG
		// std::cout << std::endl << "in: handleClientRequest:: _server.getRoot(): " << _server.getRoot() << "and path: " << path << std::endl;
		
		if (path == "/")
			filePath = this->_server.getRoot() + "/index.html";

		std::ifstream file(filePath);
		if (file.is_open())
		{
			std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			std::ostringstream response;
			response << "HTTP/1.1 200 OK\r\n";
			response << "Content-Length: " << fileContents.size() << "\r\n";
			response << "\r\n";
			response << fileContents;

			send(this->_socket, response.str().c_str(), response.str().size(), 0);
		}
		else
		{
			const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
			send(this->_socket, notFoundResponse, strlen(notFoundResponse), 0);
		}
	}

	// POST request
	else if (method == "POST")
	{
		const char* postResponse = "HTTP/1.1 200 OK\r\nContent-Length: 7\r\n\r\nPOST OK";
		send(this->_socket, postResponse, strlen(postResponse), 0);
	}

	// DELETE request
	else if (method == "DELETE")
	{
		std::string filePath = this->_server.getRoot() + path;

		if (std::remove(filePath.c_str()) == 0)
		{
			const char* deleteResponse = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nFile Deleted";
			send(this->_socket, deleteResponse, strlen(deleteResponse), 0);
		}
		else
		{
			const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
			send(this->_socket, notFoundResponse, strlen(notFoundResponse), 0);
		}
	}
	// any other methods not implemented
	else
	{
		const char* notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 19\r\n\r\n501 Not Implemented";
		send(this->_socket, notImplementedResponse, strlen(notImplementedResponse), 0);
	}
}


//The code below come from the repo  https://github.com/waltergcc/42-webserv

// void	Client::sendResponseToClient()
// {
// 	this->_sentRequest = true;
// 	this->_lastRequest = std::time(NULL);

// 	try
// 	{
// 		this->_checkRequest();
// 		std::string root = this->_server.getRoot();
// 		std::string resource = getPathWithoutSlashAtEnd(this->_resourcePath); //get the path to the ressource (file) to send to the client
// 		this->_checkLocation(root, resource, 0);
// 	}
// 	catch(const std::exception& e)
// 	{
// 		this->_writeErrorResponse(e.what());
// 	}

// 	this->_request.clear();


// 	this->_requestPayload.clear();
// 	this->_sentRequest = false;
// }


// void	Client::_checkRequest()
// {
// 	std::stringstream	ss(this->_request);

// 	this->_requestPayload.clear();
// 	this->_headers.clear();
// 	this->_checkFirstLine(ss);
// 	this->_checkAndGetHeaders(ss);
// 	this->_checkAndGetPayload(ss); //for POST
// }


// void	Client::_checkFirstLine(std::stringstream &ss)
// {
	
// 	/*
// 	check the request's first line (ex: GET /file1.txt HTTP/1.1)
// 	and get:
// 	- _ressource ()
// 	*/

// 	std::string		line;
// 	std::getline(ss, line);
// 	stringVector	parameters = getStringTokens(line, SPACE);

// 	if (parameters.size() != 3)
// 		throw std::runtime_error(RS_400);

// 	if (parameters.at(0) != GET && parameters.at(0) != DELETE && parameters.at(0) != POST)
// 		throw std::runtime_error(RS_501);
// 	this->_method = parameters.at(0);

// 	if (parameters.at(1).length() > MAX_URI_LENGHT)
// 		throw std::runtime_error(RS_414);

// 	if (parameters.at(1).find(RELATIVE_BACK) != std::string::npos || parameters.at(1) == LITERAL_BACK)
// 		throw std::runtime_error(RS_400);
// 	this->_resourcePath = urlDecode(parameters.at(1));

// 	if (parameters.at(2) == HTTP_1_0)
// 		throw std::runtime_error(RS_505);

// 	if (parameters.at(2) != HTTP_1_1)
// 		throw std::runtime_error(RS_400);
// }


// void	Client::_checkAndGetHeaders(std::stringstream &ss)
// {
// 	/*
// 	check the request second line (ex: Host: localhost)
// 	*/
	
// 	std::string line;
// 	while (std::getline(ss, line))
// 	{
// 		if (line == CARRIAGE_RETURN_STR)
// 			break;

// 		if (line.find(COLON) != std::string::npos)
// 		{
// 			std::string key = line.substr(0, line.find(COLON));
// 			std::string value = line.substr(line.find(COLON) + 1, line.find(NEWLINE));
// 			stringTrim(key, SPACES);
// 			stringTrim(value, SPACES);

// 			if (value.length() != 0 && key.length() != 0)
// 				this->_headers[key] = value;
// 			else
// 				throw std::runtime_error(RS_400);
// 		}
// 	}
// }

// void	Client::_checkAndGetPayload(std::stringstream &ss)
// {
// 	//For POST method
	
// 	// If GET or DELETE no payload is expected
// 	if (this->_method == GET || this->_method == DELETE)
// 		return;
	
// 	// For POST check if there is a Content-Length header
// 	// If not, "411 Length Required error"
// 	else if (this->_method == POST && this->_headers.find(CONTENT_LENGTH) == this->_headers.end())
// 		throw std::runtime_error(RS_411);

// 	// Get and check Content-Length header to find payload size.
// 	this->_contentLength = this->_getValidContentLength(this->_headers[CONTENT_LENGTH]);

// 	// Get and save the current reading position in ss
// 	std::streampos pos = ss.tellg();

// 	// Create a binary stringstream from the current request data to handle binary content
// 	std::stringstream binarySs(ss.str(), std::stringstream::in | std::stringstream::binary);
	
// 	// Set the read position in the binary stream to the saved position 'pos' after headers.
// 	binarySs.seekg(pos);

// 	// Resize the payload container to match the expected content length.
// 	this->_requestPayload.resize(this->_contentLength);

// 	// Read the payload content from the stream into the payload container.
// 	binarySs.read(&this->_requestPayload[0], this->_contentLength);
// }

// void Client::_checkLocation(std::string &root, std::string &resource, size_t loopCount)
// {
// 	// Check if the loop count exceeds the maximum to avoid infinite redirection loops.
// 	if (loopCount > MAX_LOOP_COUNT)
// 		throw std::runtime_error(RS_508); // Throws 508 (Loop Detected) error.

// 	// Iterate through each location in the server's location map.
// 	locationMap::const_iterator location = this->_server.getLocations().begin();
// 	for (; location != this->_server.getLocations().end(); location++)
// 	{
// 		// If the location path is a root path and the resource does not match it, skip this location.
// 		if (this->_locationIsRootAndResourceNot(location->first, resource))
// 			continue;

// 		// If the resource does not match the location and the method is not DELETE, skip this location.
// 		if (!this->_resourceHasLocation(location->first, resource) && this->_method != DELETE)
// 			continue;

// 		// Check if the HTTP method is allowed for this location. If not and it's not a valid DELETE, throw 405 (Method Not Allowed).
// 		if (!this->_methodMatches(location->second.methods))
// 		{
// 			if (!this->_hasValidDelete(resource))
// 				throw std::runtime_error(RS_405); // Throws 405 (Method Not Allowed) error.
// 		}

// 		// If a redirection is required, handle the redirection and return immediately.
// 		if (this->_hasRedirection(resource, root, loopCount, location->second.redirect, location->first))
// 			return;

// 		// Update the root path if the location has a specific root configuration.
// 		this->_updateRootIfLocationHasIt(root, location->second.root);

// 		// If the resource has a valid path within this location, stop checking other locations and return.
// 		if (this->_hasValidPath(resource, root, location->second))
// 			return;

// 		break; // If we reach here, break the loop since this location is relevant.
// 	}

// 	// After iterating, if no valid location is found, throw 403 (Forbidden).
// 	if (this->_hasInvalidLocation(location, resource))
// 		throw std::runtime_error(RS_403); // Throws 403 (Forbidden) error.

// 	// Check if the resource should be updated to include a CGI path, if applicable.
// 	this->_updateResourceWhenHasCgiPath(resource, location);

// 	// Call methodsManager to apply the necessary configurations for this location.
// 	this->_methodsManager(root, resource, location->second);
// }
