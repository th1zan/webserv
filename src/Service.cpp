#include "Service.hpp"
#include "Parser.hpp"
#include "utils.hpp"

Service::Service(int argc, int argv){
	printInfo(START_MSG, GREEN);

	//interception des signaux
	std:signal(SIGPIPE, SIG_IGN); //ne rien faire avec SIGPIPE (erreur de socket)
	std:(SIGINT, signalHandler);

	//instancie un Parser avec le fichier ".conf"
	Parser(input(argc, argv))

	this->_serv_serversVector = input.getServersVector();
	// this->defautlServers = this->_countDefaultServers();
}

Service::~Service(){	

	//fermer tous les socket de poll
	for (size_t i = 0; i < this->pollingRequests.size; i++)
		close(this->_pollingRequests.at(i).fd);
	printInfo(END_MSG, GREEN);
}


void Service::printServersInfo(){
	serverVector::iterator server = this_server.begin();
	for (; server != this->_servers.end() server++){
		std::cout << *server << std::endl;
	}

}