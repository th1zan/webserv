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
 * @brief Configures server sockets and prepares them for polling.
 * 
 * The `setup` function iterates through a vector of `Server` objects and performs
 * socket setup operations on each primary server. This involves initializing temporary
 * service information, setting socket options, binding addresses, and adding sockets 
 * to a poll socket vector for monitoring.
 * 
 * @details 
 * - Iterates through the `_serversVector` containing all server instances.
 * - Initializes temporary service information for socket setup.
 * - Skips non-primary servers to focus setup on primary ones only.
 * - Retrieves setup information such as socket, host, and port from each primary server.
 * - Sets the socket option to allow the reuse of local addresses.
 * - Converts the server's host information into a network address format.
 * - Binds the converted address to the server's socket.
 * - Configures the socket to listen for incoming connections.
 * - Adds the configured socket to the `_pollSockVec`, which is used for polling operations.
 * - Resets the temporary service information to ensure clean setup for the next server.
 * 
 */
void Service::setup()
{
	printInfo(SETUP_MSG, BLUE);
	
	std::vector<Server>::iterator server = this->_serversVector.begin();
	for(; server != this->_serversVector.end(); server++)
	{
		this->_initTmpServiceInfo();

		if (!server->getIsPrimary())
			continue;

		 //get socket, host, port
		this->_getSetupInfo(server);
		// std::cout << "TEST 2" << std::endl;

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

		this->_resetTmpServiceInfo();
	}
}

/**
 * The function initializes temporary service information (`ServiceInfo`) with specific parameters for IPv4, TCP, and
 * NULL address.
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
 * The _getSetupInfo function initializes service information based on a server's socket, host, and
 * port.
 * 
 * @param server The `server` parameter is an iterator pointing to an element in a vector of `Server`
 * objects.
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
 * @brief Sets the SO_REUSEADDR option (see documentation of 'setsockopt()') for the listening socket to allow reuse of the socket address after its closing .
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
 * @brief Binds the server address to the socket after resolving the address from the hostname.
 * 
 * This function uses the `bind()` system call to associate the socket with a local address.
 * 
 * @note The `bind()` function takes the following arguments:
 * - Socket file descriptor: An integer that uniquely identifies the socket.
 * - Address information: A structure containing the address family (IPv4 or IPv6), the local IP address, and the port number.
 * - Size of the address: The size of the address structure, which indicates how many bytes to read.
 * 
 * @see bind()
 */
void Service::_bindAddressToSocket()
{
	if (this->_tmpServiceInfo.address)
	{
		if (bind(this->_tmpServiceInfo.listeningSocketFd, this->_tmpServiceInfo.address->ai_addr, this->_tmpServiceInfo.address->ai_addrlen) < 0)
		{
			this->_resetTmpServiceInfo();
			throw std::runtime_error(ERR_BIND_SOCKET + std::string(std::strerror(errno)));
		}
	}
}

/**
 * The function _setSocketListening sets the socket to listening mode with a specified maximum number
 * of pending connections.
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
 * @brief Adds the configured socket to the polling vector for monitoring (polling).
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
