
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

bool	Client::isReadyToSend() const
{
	// the client has already sent request, so the Server is "ready to send".
	// The client has sent a request if "sentRequest = true" OR the request ends by the sequence (REQUEST_END) "\r\n\r\n"
	return (this->_sentRequest == true || this->_request.find(REQUEST_END) != std::string::npos);
}


// void	Client::sendResponse()
// {
// 	this->_sentRequest = true;
// 	this->_lastRequest = std::time(NULL);

// 	try
// 	{
// 		this->_checkRequest();
// 		std::string root = this->_server.getRoot();
// 		std::string resource = getPathWithoutSlashAtEnd(this->_resource);
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





