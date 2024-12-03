/**
 * @file Service_0.cpp
 * @brief Implementation of the Service class responsible for server management
 * and client connection handling.
 */
#include "Parser.hpp"
#include "Service.hpp"
#include "defines.hpp"
#include "utils.hpp"

/**
 * The Service constructor initializes signal handling and parses a
 * configuration file to set up server information.
 * @param argv The '.conf' file.
 * @param argc values allowed: 0 or 1.
 */
Service::Service(int argc, char **argv) {
  printInfo(START_MSG, GREEN);

  // Intercepts signals for socket handling and interruption.
  std::signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE errors for sockets.
  std::signal(SIGINT, signalHandler);

  // Instantiate Parser to process the '.conf' file.
  Parser input(argc, argv);

  this->_serversVector = input.getServersVector();

  // to make it simple at first, _nbPrimaryServers = 1
  //  this->_nbPrimaryServers = 1;
  this->_nbPrimaryServers = this->_countPrimaryServers();
}

/**
 * @brief Destructor for Service, closes all sockets in the polling vector.
 */
Service::~Service() {

  for (size_t i = 0; i < this->_pollingFdVector.size(); i++)
    close(this->_pollingFdVector.at(i).fd);
  printInfo(END_MSG, GREEN);
}

/**
 * @brief function to count the umber of primary server in the _serversVector
 * A server with an existing host and port is NOT a primary server
 *
 * @return size_t, quantity of primary servers
 */
size_t Service::_countPrimaryServers() {
  serverVector::iterator server = this->_serversVector.begin();
  size_t count = 0;

  for (; server != this->_serversVector.end(); server++) {
    if (server->getIsPrimary() == true)
      count++;
  }
  return count;
}

/**
 * @brief Print the server info
 */
void Service::printServiceInfo() {
  std::cout << "----- Service::printServiceInfo() ----" << std::endl;
  std::cout << "Host: " << _tmpServiceInfo.host << std::endl;
  std::cout << "Port: " << _tmpServiceInfo.port << std::endl;
  std::cout << "Listening Socket FD: " << _tmpServiceInfo.listeningSocketFd
            << std::endl;
  std::cout << "Client ID: " << _tmpServiceInfo.clientID << std::endl;
  std::cout << "Server ID: " << _tmpServiceInfo.serverID << std::endl;
  std::cout << "Connection Socket FD: " << _tmpServiceInfo.connectionSocketFd
            << std::endl;
  std::cout << "Poll ID: " << _tmpServiceInfo.pollID << std::endl;
  std::cout << "Mode: " << _tmpServiceInfo.mode << std::endl;
  std::cout << "Launch: " << (_tmpServiceInfo.launch ? "true" : "false")
            << std::endl;

  // Affichage des détails d'addrinfo (parameters)
  std::cout << "Parameters (addrinfo):" << std::endl;
  std::cout << "  Family: " << _tmpServiceInfo.parameters.ai_family
            << std::endl;
  std::cout << "  Socket Type: " << _tmpServiceInfo.parameters.ai_socktype
            << std::endl;
  std::cout << "  Protocol: " << _tmpServiceInfo.parameters.ai_protocol
            << std::endl;
  std::cout << "  AddrLen: " << _tmpServiceInfo.parameters.ai_addrlen
            << std::endl;
  // std::cout << "  ai_addr.sa_family: " <<
  // _tmpServiceInfo.parameters.ai_addr->sa_family << std::endl; std::cout << "
  // ai_addr.sa_len: " << _tmpServiceInfo.parameters.ai_addr->sa_len <<
  // std::endl; std::cout << "  Canonical name: " <<
  // _tmpServiceInfo.parameters.ai_canonname << std::endl;
  std::cout << "  Flags: " << _tmpServiceInfo.parameters.ai_flags << std::endl;
  std::cout << "----- /END Service::printServiceInfo() ----" << std::endl;
  std::cout << std::endl;
}
