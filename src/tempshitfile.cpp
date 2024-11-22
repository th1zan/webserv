void Service::_checkRequestedServer()
{
    // 1. Get the client's request (to know who is the requested server)
    std::string request = this->_clientVector.at(this->_tmpServiceInfo.clientID).getRequest();
    std::string requestedHost;
    size_t pos;

    // 2. Get the "host key" in the request
    if ((pos = request.find(REQUEST_HOST)) != std::string::npos)
    {
        // 3. Get the server name in the "host key"
        requestedHost = request.substr(pos + std::strlen(REQUEST_HOST));
        if ((pos = requestedHost.find(CURSOR_NEWLINE)) != std::string::npos)
            requestedHost = requestedHost.substr(0, pos);
    }
    else
    {
        // If REQUEST_HOST is not found, exit the function
        return;
    }

    // Remove port number from the server name
    if ((pos = requestedHost.find(":")) != std::string::npos)
        requestedHost = requestedHost.substr(0, pos);

    // Get the server associated with the client by default
    Server defaultServer = this->_clientVector.at(this->_tmpServiceInfo.clientID).getServer();

    // DEBUG
    std::vector<std::string> serverNames = defaultServer.getServerNameVector();
    if (std::find(serverNames.begin(), serverNames.end(), requestedHost) != serverNames.end()) {
        // DEBUG
        std::cout << "in '_checkRequestedServer':: VALID client's default host: '"
                  << requestedHost << "' is found in default server's names." << std::endl;
        return; // Already using the correct server, exit
    } else {
        std::cout << "Requested host '" << requestedHost << "' not found in default server's names." << std::endl;
    }

    // DEBUG
    std::cout << "in '_checkRequestedServer':: UPDATE the client's server: " << requestedHost << std::endl;

    // Loop on each server in the `_serversVector`
    std::vector<Server>::iterator itServer = this->_serversVector.begin();
    for (; itServer != this->_serversVector.end(); ++itServer) {
        Server tmp = *itServer;

        // Store the server names in a local variable
        std::vector<std::string> serverNames = tmp.getServerNameVector();

        // DEBUG
        std::cout << "Checking server current server names: ";
        for (std::vector<std::string>::const_iterator nameIt = serverNames.begin();
             nameIt != serverNames.end(); ++nameIt) {
            std::cout << "'" << *nameIt << "' ";
        }
        std::cout << std::endl;

        if (std::find(serverNames.begin(), serverNames.end(), requestedHost) != serverNames.end()) {
            _clientVector.at(_tmpServiceInfo.clientID).changeServer(*itServer);

            // DEBUG
            std::cout << "FOUND a matching server for the requested host: '" << requestedHost << "'" << std::endl;
            return;
        }
    }
    // DEBUG
    std::cout << "No matching server found for the requested host: '" << requestedHost << "'" << std::endl;
}