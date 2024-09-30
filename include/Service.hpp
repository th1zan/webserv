#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "defines.hpp"
#include "Parser.hpp"
#include "Client.hpp"

class Service
{
	private:
			serverVector	_servers;
			clientVector	_clients;
			pollfdVector	_pollingRequests;
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