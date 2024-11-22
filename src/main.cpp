/**
 * @file main.cpp
 * @brief The main:
 * 1) Create an instance of a Service (the webserv) using the configuration file's datas. The service handle the differents servers.
 * 2) Setup the Service (basically creates a listening socket for each server).
 * 3) Launch the Service (basically responds to the HTTP request from the Client)
 * 
 */

#include "webserv.hpp"


int main(int argc, char **argv)
{
	try
	{
		Service webserv(argc, argv);		
		webserv.setup();
		webserv.launch();
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << "Error :\t" << e.what() << RESET << std::endl;
		return (EXIT_FAILURE);
	}
}
