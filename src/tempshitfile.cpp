void Service::_checkRequestedServer()
{
	std::string request = this->_clientVector.at(this->_tmpServiceInfo.clientID).getRequest();
	std::string requestedServer;
	size_t pos;

	if ((pos = request.find(REQUEST_HOST)) != std::string::npos)
	{
		requestedServer = request.substr(pos + std::strlen(REQUEST_HOST));
		if ((pos = requestedServer.find(CURSOR_NEWLINE)) != std::string::npos)
			requestedServer = requestedServer.substr(0, pos);
	}
	else
		return;

	if ((pos = requestedServer.find(":")) != std::string::npos)
		requestedServer = requestedServer.substr(0, pos);

	Server defaultServer = this->_clientVector.at(this->_tmpServiceInfo.clientID).getServer();
	if (std::find(defaultServer.getServerNameVector().begin(), defaultServer.getServerNameVector().end(), requestedServer) != defaultServer.getServerNameVector().end())
		return;

	for (std::vector<Server>::iterator server = this->_serversVector.begin(); server != this->_serversVector.end(); ++server)
	{
		if (std::find(server->getServerNameVector().begin(), server->getServerNameVector().end(), requestedServer) != server->getServerNameVector().end()
			&& server->getHost() == defaultServer.getHost())
		{
			this->_clientVector.at(this->_tmpServiceInfo.clientID).changeServer(*server);
		}
	}
}