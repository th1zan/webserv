/**
 * @file Service.hpp
 * @brief Header file for the Service class and serviceInfo structure.
 *
 * This file contains the declaration of the Service class, which manages
 * server-client interactions and service setup.
 */

#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "defines.hpp"
#include "Parser.hpp"
#include "Client.hpp"


/**
 * @struct serviceInfo
 * @brief Holds the configuration parameters for the service.
 *
 * This structure contains the parameters necessary for establishing
 * network connections, including address information and socket file descriptors.
 */
struct serviceInfo
{
	addrinfo	parameters;
	addrinfo	*address;
	std::string	host;
	std::string	port;
	int			listeningSocketFd; //to listen - for server
	int			clientID; 
	int			serverID;
	int			connectionSocketFd; //to connect - for client
	int			pollID;
	short		mode;
	bool		launch;
};



/**
 * @note
 * The addrinfo structure is used for addressing information in socket programming.
 * It contains various fields for managing socket connections, including options,
 * address family, socket type, and more.
 * 
 * struct addrinfo {
	int              ai_flags;      // options
	int              ai_family;     // adress family (ipv4, ipv6) (AF_INET, AF_INET6, etc.)
	int              ai_socktype;   // kind of socket (SOCK_STREAM, SOCK_DGRAM, etc.)
	int              ai_protocol;   // protocol (IPPROTO_TCP, IPPROTO_UDP, etc.)
	size_t           ai_addrlen;    // adress size
	struct sockaddr *ai_addr;       // socket addr
	char            *ai_canonname;  // canonical nam
	struct addrinfo *ai_next;       // pointer to the nex struct
};
 */

/**
 * @class Service
 * @brief Manages server-client interactions and service setup.
 *
 * The Service class is responsible for initializing services, managing
 * connections, and handling client requests.
 */
class Service
{
	private:
			std::vector<Server>		_serversVector;
			std::vector<Client>		_clientVector;
			std::vector<pollfd>		_pollingFdVector;
			size_t					_defaultServers;
			serviceInfo				_tmpServiceInfo;

			void					_initServiceInfo();
			void					_getSetupInfo(std::vector<Server>::iterator server);
			void					_setReuseableAddress();
			void					_resetTmpServiceInfo();
			void					_convertHostToAddress();
			void					_bindAddressToSocket();
			void					_setSocketListening();
			void					_addSocketToPollSockVec();
			void					_launch();
			void					_initPollingVector();
			void					_pollingManager();
			void					_getLaunchInfo(int const i);
			int						_getServerIndex();
			bool					_hasDataToRead();
			bool					_isServerSocket();
			void					_acceptConnection();
			void					_readDataFromClient();
			void					_closeConnection(std::string const &msg);
			bool					_hasBadRequest();
			void					_sendDataToClient();
			void					_checkRequestedServer();


	public:
			Service(int argc, char **argv);
			~Service();

			void					setup();
			void					launch();
			void					printServersInfo();
			void					printServiceInfo();
	
};

#endif
