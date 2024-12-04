/**
 * @file Service_2_launch.cpp
 * @brief Implementation of the Service class responsible for server management
 * and client connection handling.
 */

#include "Client.hpp"
#include "Parser.hpp"
#include "Service.hpp"
#include "defines.hpp"
#include "utils.hpp"

/**
 * The `launch` function continuously initializes and polls a list of sockets
 * until a shutdown signal is received.
 */
void Service::launch() {
  while (g_shutdown == false) {
    // init the list (table) of polling sockets
    this->_initPollingVector();

    // loop on each socket of the list to check if there is a signal (something
    // to read, send, error). Note: each Service (aka Server bloc) has a socket
    this->_pollingManager();
  }
}

/**
 * The function _initPollingVector() uses poll() to survey file descriptors
 * without blocking the main process and throws an error if the poll operation
 * fails.
 */
void Service::_initPollingVector() {
  // poll() is used to survey file descriptors (=sockets) without blocking the
  // main process. It sends a signal when a socket is ready to read, listen or
  // get an error
  if (poll(this->_pollingFdVector.data(), this->_pollingFdVector.size(),
           POLL_TIME_OUT) < 0 &&
      g_shutdown == false)
    throw std::runtime_error(ERR_POLL_FAIL + std::string(std::strerror(errno)));
}

/**
 * The function `_pollingManager` iterates through a vector of socket file
 * descriptors, resets temporary service information, retrieves launch
 * information, handles errors, and performs actions based on the socket mode
 * (POLLIN or POLLOUT).
 */
void Service::_pollingManager() {
  // Loop on each socket in the polling vector containign the socket's fd
  for (size_t i = 0; i < this->_pollingFdVector.size(); i++) {
    this->_resetTmpServiceInfo();
    this->_getLaunchInfo(i);

    // Handle errors first
    if (this->_hasBadRequest())
      continue;

    // Check if there's data to read (POLLIN) or data to send (POLLOUT)
    if (this->_tmpServiceInfo.mode & POLLIN) {
      if (this->_isServerSocket()) // If SERVER's socket, accept connection
      {
        this->_acceptConnection();
      } else // If CLIENT's socket, read data
      {
        try{
          this->_readDataFromClient();
        }
        catch(const std::runtime_error& e){
           std::cerr << "Error occurred: " << e.what() << std::endl;
           this->_closeConnection("Closing connection due to error.");
        }
       }
    } else if (this->_tmpServiceInfo.mode &
               POLLOUT) // Ready to send data (POLLOUT)
    {
      this->_sendDataToClient();
    }
  }
}

/**
 * @brief  @brief Retrieves launch-specific information for each socket in the
 * polling vector.
 *
 * For example: assigns a unique client ID to the socket based on its index in
 * the polling vector.
 *
 * The client ID is calculated by subtracting the number of primary servers
 * (listening sockets) from the current index in the polling vector. This is
 * because the first `nbPrimaryServers` entries in the polling vector correspond
 * to listening sockets, and the subsequent entries represent client
 * connections. Therefore, the client IDs start after the primary servers'
 * indices and provide a zero-based index for client connections.
 *
 * @param i Index of the socket in the polling vector.
 */
void Service::_getLaunchInfo(int const i) {
  this->_tmpServiceInfo.pollID = i;
  this->_tmpServiceInfo.clientID = i - this->_nbPrimaryServers;
  this->_tmpServiceInfo.listeningSocketFd = this->_pollingFdVector.at(i).fd;
  this->_tmpServiceInfo.mode = this->_pollingFdVector.at(i).revents;
  this->_tmpServiceInfo.serverID = this->_getServerIndex();
  this->_tmpServiceInfo.launch = true;
}

/**
 * @brief Checks if the socket is a server socket.
 * @return True if socket belongs to a server, false otherwise.
 */
bool Service::_isServerSocket() {
  // the socket can be a server or a client socket. We compare the socket to the
  // server's list sockets to know if it's a server or a client
  std::vector<Server>::iterator server = this->_serversVector.begin();
  for (; server != this->_serversVector.end(); server++) {
    if (server->getSocket() == this->_tmpServiceInfo.listeningSocketFd)
      return true;
  }
  return false;
}

/**
 * The `_acceptConnection` function accepts a new connection on the server side,
 * creates a new socket for the client, sets it to non-blocking mode, and adds
 * the client to a vector of clients associated with the server.
 */
void Service::_acceptConnection() {
  /*
          - accept() a connection on the server side means create a new
     connectionSocketFd for the client .
          - accept() return a new file descriptor
          The (listening) socket is still waiting (listening) for new client's
     connexion
  */

  this->_tmpServiceInfo.connectionSocketFd =
      accept(this->_tmpServiceInfo.listeningSocketFd, NULL, NULL);

  if (this->_tmpServiceInfo.connectionSocketFd < 0)
    throw std::runtime_error(ERR_ACCEPT_SOCKET);

  // fctnl() can set socket to non-blocking
  fcntl(this->_tmpServiceInfo.connectionSocketFd, F_SETFL, O_NONBLOCK);

  // push a new (instance of) Client "linked" to the server (with the serverID
  // and the socket fd) in the Client vector
  this->_clientVector.push_back(
      Client(this->_serversVector.at(this->_tmpServiceInfo.serverID),
             this->_tmpServiceInfo.connectionSocketFd));

  // update the list of socket with the first client
  this->_addSocketToPollSockVec();
}

/**
 * The function `_readDataFromClient` reads data from a client socket into the
 * buffer and appends it to a vector, with an option to close the connection if
 * no data is received.
 */
void Service::_readDataFromClient() {
  char buffer[BUFFER_SIZE] = {0};

	//the buffer is filled with the content passing to the listeningSocketFd then the content is "read" with recv()
	int		bytes;
	while ((bytes = recv(this->_tmpServiceInfo.listeningSocketFd, buffer, BUFFER_SIZE, 0)) > 0)
	{
		// Append the received data to the client's request
        this->_clientVector.at(this->_tmpServiceInfo.clientID).appendRequest(buffer, bytes);

        // DEBUG: Log the received buffer and bytes read
        std::cout << "[DEBUG] Received " << bytes << " bytes from client ID " 
                  << this->_tmpServiceInfo.clientID << ": " << std::string(buffer, bytes) << std::endl;
	}
	if (bytes == 0) // Client disconnected
    {
        std::cerr << "[INFO] Client ID " << this->_tmpServiceInfo.clientID 
                  << " disconnected." << std::endl;
        this->_closeConnection(EMPTY_MSG);
    }
    else if (bytes < 0) // Error occurred during recv
    {
        std::cerr << "[ERROR] Failed to read data from client ID " 
                  << this->_tmpServiceInfo.clientID 
                  << " (errno: " << errno << ")." << std::endl;
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "[DEBUG] Socket temporarily unavailable. Retrying..." << std::endl;
            return; // Retry mechanism for non-blocking socket
        }
		std::cerr << "[ERROR] recv failed (errno: " << errno << ")." << std::endl;
        this->_closeConnection(EMPTY_MSG);
    }
		// DEBUGG: read the request
		std::cout << "in '_readDataFromClient:: buffer : " << buffer << std::endl;
}

/**
 * @brief Checks if there is a polling error or if the socket is disconnected.
 * @return True if there is a bad request, false otherwise.
 */
bool Service::_hasBadRequest() {
  // check if the socket send an error
  if (this->_tmpServiceInfo.mode & POLLERR) // POLLERR: error condition
  {
    this->_closeConnection(POLLERR_MSG);
    return true;
  } else if (this->_tmpServiceInfo.mode & POLLHUP) // POLLHUP: hang up
  {
    this->_closeConnection(POLLHUP_MSG);
    return true;
  } else if (this->_tmpServiceInfo.mode & POLLNVAL) // POLLNVAL: invalid request
  {
    this->_closeConnection(POLLNVAL_MSG);
    return true;
  }
  return false;
}

/**
 * @brief Sends response data to the client.
 */
void Service::_sendDataToClient() {
  // check the timeout (= if the connection and the server are fast enough)
  if (this->_clientVector.at(this->_tmpServiceInfo.clientID).isTimeout()) {
    this->_closeConnection(TIMEOUT_MSG);
    return;
  }

  // check if the client is ready to receive datas from server (=has finish to
  // send)
  if (!this->_clientVector.at(this->_tmpServiceInfo.clientID)
           .clientIsReadyToReceive())
    return;

  // chek if the server is available to send the request
  this->_checkRequestedServer();

  // send
  //  this->_clientVector.at(this->_tmpServiceInfo.clientID).sendResponseToClient();
  this->_clientVector.at(this->_tmpServiceInfo.clientID).handleClientRequest();

  this->_closeConnection(EMPTY_MSG);
}

/**
 * @brief Validates the server requested by the client in its HTTP request and
 * ensures it is correctly associated.
 *
 * This function checks if the server specified in the client's HTTP request
 * (`Host` header) exists within the available servers. It performs the
 * following steps:
 *
 * 1. Extracts the `Host` (domain name or IP address) from the client's HTTP
 * request.
 * 2. Removes any port number appended to the host, if present.
 * 3. Compares the extracted host with the names of the default server
 * associated with the client.
 *    - If the requested host matches one of the default server's names, the
 * function exits as no change is needed.
 * 4. If the host does not match the default server, iterates over all servers
 * in `_serversVector` to find a matching server name.
 *    - If a match is found, updates the client's associated server to the
 * matching server.
 * 5. If no matching server is found, the client's association remains
 * unchanged.
 *
 * This ensures that the client is connected to the correct server based on the
 * host specified in their request.
 */
void Service::_checkRequestedServer() {
  // 1. Get the client's request (to know who is the requested server)
  std::string request =
      this->_clientVector.at(this->_tmpServiceInfo.clientID).getRequest();
  std::string requestedHost;
  size_t pos;

  // 2. Get the "host key" in the request
  if ((pos = request.find(REQUEST_HOST)) != std::string::npos) {
    // 3. Get the server name in the "host key"
    requestedHost = request.substr(pos + std::strlen(REQUEST_HOST));
    if ((pos = requestedHost.find(CURSOR_NEWLINE)) != std::string::npos)
      requestedHost = requestedHost.substr(0, pos);
  } else {
    // If REQUEST_HOST is not found, exit the function
    return;
  }

  // Remove port number from the server name
  if ((pos = requestedHost.find(":")) != std::string::npos)
    requestedHost = requestedHost.substr(0, pos);

  // Get the server associated with the client by default
  Server defaultServer =
      this->_clientVector.at(this->_tmpServiceInfo.clientID).getServer();

  // Find if the requested host from the HTTP request corresponds to one of the
  // server's names
  std::vector<std::string> serverNames = defaultServer.getServerNameVector();
  // DEBUG
  std::cout << "in _checkRequestedServer:: defaultServer.serverNames: ";
  printVector(serverNames);
  if (std::find(serverNames.begin(), serverNames.end(), requestedHost) !=
      serverNames.end()) {
    return; // Already using the correct server, exit
  }

  // If it does not correspond, loop on each server in the `_serversVector` to
  // find the correct server's name
  std::vector<Server>::iterator itServer = this->_serversVector.begin();
  for (; itServer != this->_serversVector.end(); ++itServer) {
    Server tmp = *itServer;
    // DEBUG
    std::cout << "in _checkRequestedServer::change defaultServer.serverNames: ";
    printVector(serverNames);
    // Store the server names in a local variable
    std::vector<std::string> serverNames = tmp.getServerNameVector();

    if (std::find(serverNames.begin(), serverNames.end(), requestedHost) !=
        serverNames.end()) {
      _clientVector.at(_tmpServiceInfo.clientID).changeServer(*itServer);

      return;
    }
  }
}

/**
 * @brief Close connection and delete socket's file descriptor
 */
void Service::_closeConnection(std::string const &msg) {
  close(this->_tmpServiceInfo.listeningSocketFd);
  this->_pollingFdVector.erase(this->_pollingFdVector.begin() +
                               this->_tmpServiceInfo.pollID);
  this->_clientVector.erase(this->_clientVector.begin() +
                            this->_tmpServiceInfo.clientID);
  if (!msg.empty())
    printInfo(msg, RED);
}

/**
 * @brief Determines the index of the server associated with the current socket
 * in the servers vector.
 *
 * This function iterates through the `_serversVector` to find the server whose
 * listening socket matches the socket file descriptor stored in
 * `_tmpServiceInfo.listeningSocketFd`. The index of this server in the vector
 * is returned.
 *
 * @return The index of the matching server within the `_serversVector` as an
 * integer. Returns 0 if no match is found.
 *
 * @note This function assumes that each server has a unique listening socket,
 * and it is used to identify servers in contexts where multiple servers are
 * managed by the service.
 */
int Service::_getServerIndex() {
  // get un SERVER index for for each SERVER serviceInfo. There is a CLIENT
  // index for CLIENT Service Info. To know if the ServiceInfo is a SERVER, we
  // compare its socket to the socket in ServersVector.
  std::vector<Server>::iterator server = this->_serversVector.begin();
  for (; server != this->_serversVector.end(); server++) {
    if (server->getSocket() == this->_tmpServiceInfo.listeningSocketFd) {

      return static_cast<int>(server - this->_serversVector.begin());
    }
  }
  return 0;
}
