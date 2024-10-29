#include "Service.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "defines.hpp"

Service::Service(int argc, char **argv){
	printInfo(START_MSG, GREEN);

	//interception des signaux
	std::signal(SIGPIPE, SIG_IGN); //ne rien faire avec SIGPIPE (erreur de socket)
	std::signal(SIGINT, signalHandler);

	//instancie un Parser avec le fichier ".conf"
	Parser	input(argc, argv);

	this->_serversVector = input.getServersVector();

	//to make it simple at first, _defaultServers = 1
	this->_defaultServers = 1;
	// this->_defaultServers = this->_countDefaultServers();
}

Service::~Service(){	

	//fermer tous les socket de poll
	for (size_t i = 0; i < this->_pollingFdVector.size(); i++)
		close(this->_pollingFdVector.at(i).fd);
	printInfo(END_MSG, GREEN);
}


// Setup functions //
void Service::setup()
{
	// printInfo(SETUP_MSG, BLUE);

	this->_initServiceInfo();

	std::vector<Server>::iterator server = this->_serversVector.begin();
	for(; server != this->_serversVector.end(); server++)
	{
		
		server->printServers();
		// if (!server->getIsDefault())
		// 	continue;
		// std::cout << "TEST 1" << std::endl;

		this->_getSetupInfo(server);
		// std::cout << "TEST 2" << std::endl;

		this->printServiceInfo();
		this->_setReuseableAddress();
		// std::cout << "TEST 3" << std::endl;

		this->_convertHostToAddress();
		// std::cout << "TEST 4" << std::endl;

		this->_bindAddressToSocket();
		// std::cout << "TEST 5" << std::endl;

		this->_setSocketListening();
		// std::cout << "TEST 6" << std::endl;
		
		this->_addSocketToPollSockVec();
		// std::cout << "TEST 7" << std::endl;

	// 	printInfo(SET_SERVER_MSG(this->_tmp.host, this->_tmp.port), BLUE);

		this->_resetTmpServiceInfo();
	}
	
}

void Service::_initServiceInfo()
{
	std::memset(&this->_tmpServiceInfo.parameters, 0, sizeof(this->_tmpServiceInfo.parameters));
	this->_tmpServiceInfo.parameters.ai_family = AF_INET;			// IPv4
	this->_tmpServiceInfo.parameters.ai_socktype = SOCK_STREAM;	// TCP
	this->_tmpServiceInfo.parameters.ai_protocol = IPPROTO_TCP;	// TCP
	this->_tmpServiceInfo.address = NULL;
}

void Service::_getSetupInfo(std::vector<Server>::iterator server)
{
	server->createSocket();
	this->_tmpServiceInfo.listeningSocketFd = server->getSocket();
	this->_tmpServiceInfo.host = server->getHost();
	this->_tmpServiceInfo.port = server->getPort();
	this->_tmpServiceInfo.launch = false;
}

void Service::_setReuseableAddress()
{
	int active = 1;

	//set socket's options (see documentation of 'setsockopt'). SO_REUSEADDR allow to re-use the socket after its closing 
	if (setsockopt(this->_tmpServiceInfo.listeningSocketFd, SOL_SOCKET, SO_REUSEADDR, &active, sizeof(int)) < 0)
	{
		this->_resetTmpServiceInfo();
		throw std::runtime_error(ERR_SET_SOCKET + std::string(std::strerror(errno)));
	}
}


void Service::_resetTmpServiceInfo()
{
	if (this->_tmpServiceInfo.address)
	{
		freeaddrinfo(this->_tmpServiceInfo.address);
		this->_tmpServiceInfo.address = NULL;
	}
	std::memset(&this->_tmpServiceInfo.parameters, 0, sizeof(this->_tmpServiceInfo.parameters));
	this->_tmpServiceInfo.host.clear();
	this->_tmpServiceInfo.port.clear();
	this->_tmpServiceInfo.pollID = 0;
	this->_tmpServiceInfo.clientID = 0;
	this->_tmpServiceInfo.serverID = 0;
	this->_tmpServiceInfo.listeningSocketFd = 0;
	this->_tmpServiceInfo.mode = 0;
	this->_tmpServiceInfo.connectionSocketFd = 0;
	this->_tmpServiceInfo.launch = false;
}

void Service::_convertHostToAddress()
{
	
	//convert hostname to IP adress for sockets (see documentation of 'getaddrinfo')
	if (getaddrinfo(this->_tmpServiceInfo.host.c_str(), this->_tmpServiceInfo.port.c_str(), &this->_tmpServiceInfo.parameters, &this->_tmpServiceInfo.address) != 0)
	{
		this->_resetTmpServiceInfo();
		throw std::runtime_error(ERR_GET_ADDR_INFO + std::string(std::strerror(errno)));
	}
}


void Service::_bindAddressToSocket()
{
	//bind host's adress to the socket (after getting the address from the hostname)
	if (this->_tmpServiceInfo.address)
	{
		if (bind(this->_tmpServiceInfo.listeningSocketFd, this->_tmpServiceInfo.address->ai_addr, this->_tmpServiceInfo.address->ai_addrlen) < 0)
		{
			this->_resetTmpServiceInfo();
			throw std::runtime_error(ERR_BIND_SOCKET + std::string(std::strerror(errno)));
		}
	}
}

void Service::_setSocketListening()
{
	//set the socket for listening
	if (listen(this->_tmpServiceInfo.listeningSocketFd, MAX_PENDING) < 0)
	{
		this->_resetTmpServiceInfo();
		throw std::runtime_error(ERR_LISTEN_SOCKET + std::string(std::strerror(errno)));
	}
}


void Service::_addSocketToPollSockVec()
{
	pollfd pollSocket;

	if (this->_tmpServiceInfo.launch == true) //we are surveying a Client socket
	{
		//the 'connection Socket' is set to listen or read (POLLIN | POLLOUT)" 
		pollSocket.fd = this->_tmpServiceInfo.connectionSocketFd;
		pollSocket.events = POLLIN | POLLOUT;
	}
	else //we are surveying a Server socket
	{
		//the 'listeningSocketFd' is  set to listen (POLLIN)" (aka wait for new connections )
		pollSocket.fd = this->_tmpServiceInfo.listeningSocketFd;
		pollSocket.events = POLLIN;
	}
	pollSocket.revents = 0;
	//the socket is added to the poll
	this->_pollingFdVector.push_back(pollSocket);
}

//********* Launch functions */
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

void Service::_initPollingVector()
{
	//poll() is used to survey file descriptors (=sockets) without blocking the main process. It sends a signal when a socket is ready to read, listen or get an error
	if (poll(this->_pollingFdVector.data(), this->_pollingFdVector.size(), POLL_TIME_OUT) < 0 && g_shutdown == false)
		throw std::runtime_error(ERR_POLL_FAIL + std::string(std::strerror(errno)));
}


//!!! OLD FASHIONNED
// void Service::_pollingManager()
// {
// 	//loop on each service (each socket)
// 	for (size_t i = 0; i < this->_pollingFdVector.size(); i++)
// 	{
		
// 		this->_resetTmpServiceInfo();
// 		//get the info of the current (i) server
// 		this->_getLaunchInfo(i);

// 		if (this->_hasDataToRead())
// 			continue;
// 		if (this->_hasBadRequest())
// 			continue;
// 		if (this->_isServerSocket()) //if server socket -> accept connection, if client socket->readData 
// 			continue;

// 		this->_sendDataToClient();
// 	}
// }

void Service::_pollingManager()
{
	// Loop on each socket in the polling vector containign the socket's fd
	for (size_t i = 0; i < this->_pollingFdVector.size(); i++)
	{
		this->_resetTmpServiceInfo();
		this->_getLaunchInfo(i);

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
				std::cout << "read data from client " << std::endl;
			}
		}
		else if (this->_tmpServiceInfo.mode & POLLOUT) // Ready to send data (POLLOUT)
		{
			this->_sendDataToClient();

			// DEBUGG
			std::cout << "read data from client " << std::endl;
			// std::cout << "in '_readDataFromClient:: this->_tmpServiceInfo.clientID : " << this->_tmpServiceInfo.clientID << std::endl;
		}
		// DEBUG
		// std::cout << "loop in polling manager" << std::endl;
	}
}

void Service::_getLaunchInfo(int const i)
{
	
	this->_tmpServiceInfo.pollID = i;
	this->_tmpServiceInfo.clientID = i - this->_defaultServers;
	this->_tmpServiceInfo.listeningSocketFd = this->_pollingFdVector.at(i).fd;
	this->_tmpServiceInfo.mode = this->_pollingFdVector.at(i).revents;
	this->_tmpServiceInfo.serverID = this->_getServerIndex();
	this->_tmpServiceInfo.launch = true;
}

//OLD FASHIONNED
// bool Service::_hasDataToRead()
// {
// 	//if the socket has data to read then:
// 	if (this->_tmpServiceInfo.mode & POLLIN) //bitwise comparison -> POLLIN is present but POLLOUT could be present (or not)
// 	{
// 		// if tmpServiceInfo is a SERVER (we compare the socket to the server's socket list) -> we acceptConnection(). 
// 		if (this->_isServerSocket())
// 			this->_acceptConnection();
// 		else // so tmpServiceInfo is a CLIENT's socket and we readData))
// 			this->_readDataFromClient();
// 		return true;
// 	}
// 	return false;
// }

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


	//if the buffer is not empty, its content is append to the Client's _request variable 
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
	// no change if the requested server is the same as the default server
	if (requestedServer == defaultServer.getServerName())
		return;

	// Go through the list of servers
	std::vector<Server>::iterator server = this->_serversVector.begin();
	for (; server != this->_serversVector.end(); server++)
	{
		// If a server name and host is found, update the client's server.
		if (requestedServer == server->getServerName() && server->getHost() == defaultServer.getHost())
			this->_clientVector.at(this->_tmpServiceInfo.clientID).changeServer(*server);
	}
}



void Service::_closeConnection(std::string const &msg)
{
	close(this->_tmpServiceInfo.listeningSocketFd);
	this->_pollingFdVector.erase(this->_pollingFdVector.begin() + this->_tmpServiceInfo.pollID);
	this->_clientVector.erase(this->_clientVector.begin() + this->_tmpServiceInfo.clientID);
	if (!msg.empty())
		printInfo(msg, RED);
}



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



// void Service::printServersInfo(){
// 	serverVector::iterator server = this_server.begin();
// 	for (; server != this->_servers.end() server++){
// 		std::cout << *server << std::endl;
// 	}

// }


void Service::printServiceInfo(){
	std::cout << "----- Service::printServiceInfo() ----" << std::endl;
	std::cout << "Host: " << _tmpServiceInfo.host << std::endl;
	std::cout << "Port: " << _tmpServiceInfo.port << std::endl;
	std::cout << "Listening Socket FD: " << _tmpServiceInfo.listeningSocketFd << std::endl;
	std::cout << "Client ID: " << _tmpServiceInfo.clientID << std::endl;
	std::cout << "Server ID: " << _tmpServiceInfo.serverID << std::endl;
	std::cout << "Connection Socket FD: " << _tmpServiceInfo.connectionSocketFd << std::endl;
	std::cout << "Poll ID: " << _tmpServiceInfo.pollID << std::endl;
	std::cout << "Mode: " << _tmpServiceInfo.mode << std::endl;
	std::cout << "Launch: " << (_tmpServiceInfo.launch ? "true" : "false") << std::endl;
	
	// Affichage des détails d'addrinfo (parameters)
	std::cout << "Parameters (addrinfo):" << std::endl;
	std::cout << "  Family: " << _tmpServiceInfo.parameters.ai_family << std::endl;
	std::cout << "  Socket Type: " << _tmpServiceInfo.parameters.ai_socktype << std::endl;
	std::cout << "  Protocol: " << _tmpServiceInfo.parameters.ai_protocol << std::endl;
	std::cout << "  Flags: " << _tmpServiceInfo.parameters.ai_flags << std::endl;
	std::cout << "----- /END Service::printServiceInfo() ----" << std::endl;
	std::cout << std::endl;

}
