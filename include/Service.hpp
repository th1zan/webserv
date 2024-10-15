#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "defines.hpp"
#include "Parser.hpp"
#include "Client.hpp"

class Service
{
	private:
			std::vector<Server>	_serversVector;
			std::vector<Client>	_clients;
			std::vector<pollfd>	_pollingRequests;
			size_t			_defaultServers;
			serviceInfo		_tmp;

	public:
			Service(int argc, char *argv);
			~Service();

			void	setup();
			void	launch();
			void	printServersInfo();
	
};

#endif