/**
 * @file Service_0.cpp
 * @brief Implementation of the Service class responsible for server management and client connection handling.
 */

#include "Service.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "defines.hpp"

/**
 * @brief Constructs a Service object, sets up signal handling, and parses the configuration file.
 * @param argv '.conf' file.
 * @param argc 0 or 1.
 */
Service::Service(int argc, char **argv){
	printInfo(START_MSG, GREEN);

	 // Intercepts signals for socket handling and interruption.
	std::signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE errors for sockets.
	std::signal(SIGINT, signalHandler);

	// Instantiate Parser to process the '.conf' file.
	Parser	input(argc, argv);

	this->_serversVector = input.getServersVector();

	//to make it simple at first, _nbPrimaryServers = 1
	// this->_nbPrimaryServers = 1;
	this->_nbPrimaryServers = this->_countPrimaryServers();
	
	//DEBUG
	// std::cout << "in Service constructor:: this->_nbPrimaryServers: " << this->_nbPrimaryServers << std::endl;


}

/**
 * @brief Destructor for Service, closes all sockets in the polling vector.
 */
Service::~Service(){	

	for (size_t i = 0; i < this->_pollingFdVector.size(); i++)
		close(this->_pollingFdVector.at(i).fd);
	printInfo(END_MSG, GREEN);
}

/**
 * @brief function to count the the primary server in the _serversVector
 * A server with an existing host and port is NOT a primary server
 * 
 * @return size_t, quantity of primary servers
 */
size_t Service::_countPrimaryServers(){
	serverVector::iterator server = this->_serversVector.begin();
	size_t count = 0;

	for (; server != this->_serversVector.end(); server++)
	{
		if (server->getIsPrimary() == true)
			count++;
	}
	return count;
}

/**
 * @brief Print the server info
 */
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
	std::cout << "  AddrLen: " << _tmpServiceInfo.parameters.ai_addrlen << std::endl;
	// std::cout << "  ai_addr.sa_family: " << _tmpServiceInfo.parameters.ai_addr->sa_family << std::endl;
	// std::cout << "  ai_addr.sa_len: " << _tmpServiceInfo.parameters.ai_addr->sa_len << std::endl;	
	// std::cout << "  Canonical name: " << _tmpServiceInfo.parameters.ai_canonname << std::endl;
	std::cout << "  Flags: " << _tmpServiceInfo.parameters.ai_flags << std::endl;
	std::cout << "----- /END Service::printServiceInfo() ----" << std::endl;
	std::cout << std::endl;

}
//  * struct addrinfo {
// 	int              ai_flags;      // options
// 	int              ai_family;     // adress family (ipv4, ipv6) (AF_INET, AF_INET6, etc.)
// 	int              ai_socktype;   // kind of socket (SOCK_STREAM, SOCK_DGRAM, etc.)
// 	int              ai_protocol;   // protocol (IPPROTO_TCP, IPPROTO_UDP, etc.)
// 	size_t           ai_addrlen;    // adress size
// 	struct sockaddr *ai_addr;       // socket addr
// 	char            *ai_canonname;  // canonical name
// 	struct addrinfo *ai_next;       // pointer to the nex struct
