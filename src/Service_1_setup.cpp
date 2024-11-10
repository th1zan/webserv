/**
 * @file Service_1_setup.cpp
 * @brief Implementation of the Service class responsible for server management and client connection handling.
 */

#include "Service.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "defines.hpp"

/**
 * @brief Initializes the server setup, binds addresses, and adds sockets to the polling vector.
 */
void Service::setup()
{
	// printInfo(SETUP_MSG, BLUE);

	std::vector<Server>::iterator server = this->_serversVector.begin();
	for(; server != this->_serversVector.end(); server++)
	{
		this->_initTmpServiceInfo();

		//DEBUG
		// server->printServers();
		// std::cout << "in Service::setup(): server->getServerName: " << server->getServerName() << std::endl;

		if (!server->getIsPrimary())
			continue;

		//DEBUG
		// std::cout << "TEST 1" << std::endl;

		this->_getSetupInfo(server); //get socket, host, port
		// std::cout << "TEST 2" << std::endl;

	
		this->_setReuseableAddress();
		// std::cout << "TEST 3" << std::endl;

		// DEBUG
		// this->printServiceInfo();

		this->_convertHostToAddress();
		// std::cout << "TEST 4" << std::endl;

		// DEBUG
		// this->printServiceInfo();

		this->_bindAddressToSocket();
		// std::cout << "TEST 5" << std::endl;

		this->_setSocketListening();
		// std::cout << "TEST 6" << std::endl;
		
		this->_addSocketToPollSockVec();
		// std::cout << "TEST 7" << std::endl;

	//DEBUG
	// 	printInfo(SET_SERVER_MSG(this->_tmp.host, this->_tmp.port), BLUE);

		this->_resetTmpServiceInfo();
	}
	
}

/**
 * @brief Initializes temporary ServiceInfo parameters for server setup.
 */
void Service::_initTmpServiceInfo()
{
	std::memset(&this->_tmpServiceInfo.parameters, 0, sizeof(this->_tmpServiceInfo.parameters));
	this->_tmpServiceInfo.parameters.ai_family = AF_INET;			// IPv4
	this->_tmpServiceInfo.parameters.ai_socktype = SOCK_STREAM;	// TCP
	this->_tmpServiceInfo.parameters.ai_protocol = IPPROTO_TCP;	// TCP
	this->_tmpServiceInfo.address = NULL;
}

/**
 * @brief Retrieves setup information from the server and stores it in temporary variables.
 * @param server Iterator pointing to the current Server.
 */
void Service::_getSetupInfo(std::vector<Server>::iterator server)
{
	server->createSocket();
	this->_tmpServiceInfo.listeningSocketFd = server->getSocket();
	this->_tmpServiceInfo.host = server->getHost();
	this->_tmpServiceInfo.port = server->getPort();
	this->_tmpServiceInfo.launch = false;
}

/**
 * @brief Sets the SO_REUSEADDR option (see documentation of 'setsockopt()') on the listening socket to allow reuse of the socket address after its closing .
 */
void Service::_setReuseableAddress()
{
	int active = 1;
	if (setsockopt(this->_tmpServiceInfo.listeningSocketFd, SOL_SOCKET, SO_REUSEADDR, &active, sizeof(int)) < 0)
	{
		this->_resetTmpServiceInfo();
		throw std::runtime_error(ERR_SET_SOCKET + std::string(std::strerror(errno)));
	}
}

/**
 * @brief Resets temporary ServiceInfo variables for new configuration.
 */
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

/**
 * @brief Converts the hostname to an IP address for use with sockets (see documentation of 'getaddrinfo()').
 */
void Service::_convertHostToAddress()
{
	if (getaddrinfo(this->_tmpServiceInfo.host.c_str(), this->_tmpServiceInfo.port.c_str(), &this->_tmpServiceInfo.parameters, &this->_tmpServiceInfo.address) != 0)
	{
		this->_resetTmpServiceInfo();
		throw std::runtime_error(ERR_GET_ADDR_INFO + std::string(std::strerror(errno)));
	}
}

/**
 * @brief Binds the server address to the socket (after getting the address from the hostname).(see documentation of 'bind()')
 */
void Service::_bindAddressToSocket()
{
	//DEBUG
	// std::cout << "in _bindAddressToSocket::" << std::endl;
	// printServiceInfo();
	
	if (this->_tmpServiceInfo.address)
	{
		if (bind(this->_tmpServiceInfo.listeningSocketFd, this->_tmpServiceInfo.address->ai_addr, this->_tmpServiceInfo.address->ai_addrlen) < 0)
		{
			//DEBUG
			// std::cout << "in _bindAddressToSocket:: ERROR" << std::endl;
			// printServiceInfo();


			this->_resetTmpServiceInfo();
			throw std::runtime_error(ERR_BIND_SOCKET + std::string(std::strerror(errno)));
		}
	}
}

/**
 * @brief Sets the socket to listening mode.
 */
void Service::_setSocketListening()
{
	if (listen(this->_tmpServiceInfo.listeningSocketFd, MAX_PENDING) < 0)
	{
		this->_resetTmpServiceInfo();
		throw std::runtime_error(ERR_LISTEN_SOCKET + std::string(std::strerror(errno)));
	}
}

/**
 * @brief Adds the configured socket to the polling vector for monitoring.
 */
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
