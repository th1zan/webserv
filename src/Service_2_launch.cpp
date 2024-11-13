/**
 * @file Service_2_launch.cpp
 * @brief Implementation of the Service class responsible for server management and client connection handling.
 */

#include "Service.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "defines.hpp"

/**
 * @brief Begins the main polling loop to handle client connections and server actions.
 */
void Service::launch()
{
	printInfo(LAUNCH_MSG, BLUE);

	while (g_shutdown == false)
	{
		this->_initPollingVector(); //init the list (table) of polling sockets
		this->_pollingManager(); //loop on each socket of the list to check if there is a signal (something to read, send, error). Note: each Service (aka Server bloc) has a socket
		
		//DEBUG
		// std::cout << "server is launched" << std::endl;
	}
}

/**
 * @brief Initializes the polling vector by calling poll on the sockets.
 */
void Service::_initPollingVector()
{
	//poll() is used to survey file descriptors (=sockets) without blocking the main process. It sends a signal when a socket is ready to read, listen or get an error
	if (poll(this->_pollingFdVector.data(), this->_pollingFdVector.size(), POLL_TIME_OUT) < 0 && g_shutdown == false)
		throw std::runtime_error(ERR_POLL_FAIL + std::string(std::strerror(errno)));
}

/**
 * @brief Manages polling events for each socket, handling client-server interactions.
 */
void Service::_pollingManager()
{
	// Loop on each socket in the polling vector containign the socket's fd
	for (size_t i = 0; i < this->_pollingFdVector.size(); i++)
	{
		this->_resetTmpServiceInfo();
		this->_getLaunchInfo(i);

		//DEBUG
		// std::cout << "in : _pollingManager():: i: " << i << std::endl;


		// Handle errors first
		if (this->_hasBadRequest())
			continue;

		// Check if there's data to read (POLLIN) or data to send (POLLOUT)
		if (this->_tmpServiceInfo.mode & POLLIN) 
		{
			if (this->_isServerSocket()) // If SERVER's socket, accept connection
			{
				this->_acceptConnection();
				// // DEBUG
				std::cout << "accept connection" << std::endl;
			}
			else // If CLIENT's socket, read data
			{
				this->_readDataFromClient();
				// DEBUG
				// std::cout << "read data from client " << std::endl;
			}
		}
		else if (this->_tmpServiceInfo.mode & POLLOUT) // Ready to send data (POLLOUT)
		{
			this->_sendDataToClient();

			// DEBUGG
			// std::cout << "read data from client " << std::endl;
			// std::cout << "in '_readDataFromClient:: this->_tmpServiceInfo.clientID : " << this->_tmpServiceInfo.clientID << std::endl;
		}
		// DEBUG
		// std::cout << "loop in polling manager" << std::endl;
	}
}

/**
 * @brief Retrieves launch-specific information for each socket in the polling vector.
 * @param i Index of the socket in the polling vector.
 */
void Service::_getLaunchInfo(int const i)
{
	//DEBUG
	// std::cout << "in : _getLaunchInfo():: i: " << i << std::endl;
	// std::cout << "in : _getLaunchInfo():: this->_nbPrimaryServers: " << this->_nbPrimaryServers << std::endl;

	this->_tmpServiceInfo.pollID = i;
	this->_tmpServiceInfo.clientID = i - this->_nbPrimaryServers;
	this->_tmpServiceInfo.listeningSocketFd = this->_pollingFdVector.at(i).fd;
	this->_tmpServiceInfo.mode = this->_pollingFdVector.at(i).revents;
	this->_tmpServiceInfo.serverID = this->_getServerIndex();
	this->_tmpServiceInfo.launch = true;
}

/**
 * @brief Checks if the socket is a server socket.
 * @return True if socket belongs to a server, false otherwise.
 */
bool Service::_isServerSocket()
{

	//the socket can be a server or a client socket. We compare the socket to the server's list sockets to know if it's a server or a client
	std::vector<Server>::iterator server = this->_serversVector.begin();
	for (; server != this->_serversVector.end(); server++)
	{
		if (server->getSocket() == this->_tmpServiceInfo.listeningSocketFd)
			return true;
	}
	return false;
}

/**
 * @brief Accepts a new client connection on the server socket.
 */
void Service::_acceptConnection()
{
	//accept() a connection on the server side means create a new connectionSocketFd for the client .
	//accept() return a new file descriptor
	//The (listening) socket is still waiting (listening) for new client's connexion

	this->_tmpServiceInfo.connectionSocketFd = accept(this->_tmpServiceInfo.listeningSocketFd, NULL, NULL);

	if (this->_tmpServiceInfo.connectionSocketFd < 0)
		throw std::runtime_error(ERR_ACCEPT_SOCKET);
	
	fcntl(this->_tmpServiceInfo.connectionSocketFd, F_SETFL, O_NONBLOCK);	// fctnl() can set socket to non-blocking
	
	//push a new (instance of) Client "linked" to the server (with the serverID and the socket fd) in the Client vector
	this->_clientVector.push_back(Client(this->_serversVector.at(this->_tmpServiceInfo.serverID), this->_tmpServiceInfo.connectionSocketFd));

	this->_addSocketToPollSockVec(); //update the list of socket with the first client
}

/**
 * @brief Reads data from a client socket into the buffer.
 */
void Service::_readDataFromClient()
{
	
	// DEBUGG
	// std::cout << "in '_readDataFromClient:: this->_tmpServiceInfo.listeningSocketFd : " << this->_tmpServiceInfo.listeningSocketFd << std::endl;
	// printServiceInfo();

	char	buffer[BUFFER_SIZE] = {0};
	//the buffer is filled with the content passing to the listeningSocketFd then the content is "read" with recv()
	int		bytes = recv(this->_tmpServiceInfo.listeningSocketFd, buffer, BUFFER_SIZE, 0);

		// DEBUGG
		// std::cout << "in '_readDataFromClient:: buffer : " << std::endl << buffer << std::endl;
		// std::cout << "in '_readDataFromClient:: bytes : " << bytes << std::endl;


	//
	
	if (bytes > 0){
		// DEBUGG
		// std::cout << "in '_readDataFromClient:: this->_tmpServiceInfo.clientID : " << this->_tmpServiceInfo.clientID << std::endl;


		this->_clientVector.at(this->_tmpServiceInfo.clientID).appendRequest(buffer, bytes);

		// DEBUGG
		std::cout << "in '_readDataFromClient:: buffer : " << buffer << std::endl;
	}
	else
		this->_closeConnection(EMPTY_MSG);
}


/**
 * @brief Checks if there is a polling error or if the socket is disconnected.
 * @return True if there is a bad request, false otherwise.
 */
bool Service::_hasBadRequest()
{
	//check if the socket send an error
	if (this->_tmpServiceInfo.mode & POLLERR)	// POLLERR: error condition
	{
		this->_closeConnection(POLLERR_MSG);
		return true;
	}
	else if (this->_tmpServiceInfo.mode & POLLHUP)		// POLLHUP: hang up
	{
		this->_closeConnection(POLLHUP_MSG);
		return true;
	}
	else if (this->_tmpServiceInfo.mode & POLLNVAL)	// POLLNVAL: invalid request
	{
		this->_closeConnection(POLLNVAL_MSG);
		return true;
	}
	return false;
}

/**
 * @brief Sends response data to the client.
 */
void Service::_sendDataToClient()
{
	//check the timeout (= if the connection and the server are fast enough)
	if (this->_clientVector.at(this->_tmpServiceInfo.clientID).isTimeout())
	{
		this->_closeConnection(TIMEOUT_MSG);
		return;	
	}

	//check if the client is ready to receive datas from server (=has finish to send)
	if (!this->_clientVector.at(this->_tmpServiceInfo.clientID).clientIsReadyToReceive())
		return;

	//chek if the server is available to send the request
	this->_checkRequestedServer();

	//send
	// this->_clientVector.at(this->_tmpServiceInfo.clientID).sendResponseToClient();
	this->_clientVector.at(this->_tmpServiceInfo.clientID).handleClientRequest();

	this->_closeConnection(EMPTY_MSG);
}

/**
 * @brief Check if the Server asked by the client exists in serversVector.
 * More precisely: exists in one of the _serverNameVector for each server in serversVector.
 */
void Service::_checkRequestedServer()
{
	//1. Get the client's request (to know who is the requested server)
	std::string request = this->_clientVector.at(this->_tmpServiceInfo.clientID).getRequest();
	std::string requestedServer;
	size_t pos;

	// 2. Get the "host key" in the request
	if ((pos = request.find(REQUEST_HOST)))
	{
		// 3. Get the server name in the "host key"
		requestedServer = request.substr(pos + std::strlen(REQUEST_HOST));
		if ((pos = requestedServer.find(CURSOR_NEWLINE)))
			requestedServer = requestedServer.substr(0, pos);
	}
	else
		return;

	// remove port number from the server name
	if ((pos = requestedServer.find(":")))
		requestedServer = requestedServer.substr(0, pos);
	
	// Get the DEFAULT server associated with the client.
	Server defaultServer = this->_clientVector.at(this->_tmpServiceInfo.clientID).getServer();
	// no change if the requested server is the same (=could be find the list of serverNameVecor) as the default server.
	if (std::find(defaultServer.getServerNameVector().begin(), defaultServer.getServerNameVector().end(), requestedServer) != defaultServer.getServerNameVector().end())
		return;

	// Go through the list of servers
	std::vector<Server>::iterator server = this->_serversVector.begin();
	for (; server != this->_serversVector.end(); server++)
	{
		// If a server name and host is found, update the client's server.
		if (std::find(defaultServer.getServerNameVector().begin(), defaultServer.getServerNameVector().end(), requestedServer) != defaultServer.getServerNameVector().end()
			&& server->getHost() == defaultServer.getHost())
			this->_clientVector.at(this->_tmpServiceInfo.clientID).changeServer(*server);
	}
}

/**
 * @brief Close connection and delete socket's file descriptor
 */
void Service::_closeConnection(std::string const &msg)
{
	close(this->_tmpServiceInfo.listeningSocketFd);
	this->_pollingFdVector.erase(this->_pollingFdVector.begin() + this->_tmpServiceInfo.pollID);
	this->_clientVector.erase(this->_clientVector.begin() + this->_tmpServiceInfo.clientID);
	if (!msg.empty())
		printInfo(msg, RED);
}


/**
 * @brief Retrieves the server index for the current socket.
 * @return Server index as integer.
 */
int Service::_getServerIndex()
{
	//get un SERVER index for for each SERVER serviceInfo. There is a CLIENT index for CLIENT Service Info.
	//To know if the ServiceInfo is a SERVER, we compare its socket to the socket in ServersVector.
	std::vector<Server>::iterator server = this->_serversVector.begin();
	for (; server != this->_serversVector.end(); server++)
	{
		
		if (server->getSocket() == this->_tmpServiceInfo.listeningSocketFd)
			return static_cast<int>(server - this->_serversVector.begin());
	}
	return 0;
}
