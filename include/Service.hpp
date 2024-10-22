#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "defines.hpp"
#include "Parser.hpp"
#include "Client.hpp"


struct serviceInfo
{
	addrinfo	parameters;
	addrinfo	*address;
	std::string	host;
	std::string	port;
	int			listeningSocket; //to listen - for server
	int			clientID; 
	int			serverID;
	int			connectionSocket; //to connect - for client
	int			pollID;
	short		mode;
	bool		launch;
};

/*
For information:

struct addrinfo {
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

class Service
{
	private:
			std::vector<Server>	_serversVector;
			std::vector<Client>	_clientVector;
			std::vector<pollfd>	_pollingFdVector;
			size_t			_defaultServers;
			serviceInfo		_tmpServiceInfo;

			void			_initServiceInfo();
			void			_getSetupInfo(std::vector<Server>::iterator server);
			void			_setReuseableAddress();
			void			_resetTmpServiceInfo();
			void			_convertHostToAddress();
			void			_bindAddressToSocket();
			void			_setSocketListening();
			void			_addSocketToPollSockVec();
			void			_launch();
			void			_initPollingVector();
			void			_pollingManager();
			void			_getLaunchInfo(int const i);
			int				_getServerIndex();
			bool			_hasDataToRead();
			bool			_isServerSocket();
			void			_acceptConnection();
			void			_readData();
			void			_closeConnection(std::string const &msg);
			bool			_hasBadRequest();
			void			_hasDataToSend();
			void			_checkRequestedServer();


	public:
			Service(int argc, char **argv);
			~Service();

			void	setup();
			void	launch();
			void	printServersInfo();
	
};

#endif