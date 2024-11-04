/**
 * @file main.cpp
 * @brief The main create an instance of a Service (the webserv), then set it up and launch it.
 * 
 */

#include "webserv.hpp"

int main(int argc, char **argv)
{
	try
	{
		
		//DEBUG
		// std::cout << " !!! don't forget to change the path 'root /PATH_TO/..../webserv/webSites/main;' in the 'default.conf' file " << std::endl

		Service webserv(argc, argv);
		// webserv.printServerInfo();
		
		
		webserv.setup();
		webserv.launch();
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << "Error :\t" << e.what() << RESET << std::endl;
		return (EXIT_FAILURE);
	}
	
}
