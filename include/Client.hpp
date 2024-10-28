#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "defines.hpp"
#include "Server.hpp"
#include "utils.hpp"

class Client
{
	private:
		Server		_server;
		int			_socket;
		bool		_sentRequest;
		std::string	_request;
		time_t		_lastRequest;
		std::string	_resourcePath; //path to the ressource ../site/page
		std::string	_method; //GET POST DELETE

		std::string							_requestPayload;
		std::map<std::string, std::string>	_headers;

		
		//befor sending request to the Client

		void		_checkRequest();
		void		_checkFirstLine(std::stringstream &ss);
		void 		_checkAndGetHeaders(std::stringstream &ss);
		void		_checkAndGetPayload(std::stringstream &ss);
		void		_checkLocation(std::string &root, std::string &resource, size_t loopCount);
		

		Client();

	public:
		Client(Server server, int socket);
		~Client();

		void		appendRequest(char const *buffer, size_t size);
		bool		isTimeout() const;
		bool		clientIsReadyToReceive() const;
		
		
		// void		sendResponseToClient();
		void		handleClientRequest(); //Janneta's function
		


		

		const std::string& 	getRequest() const;
		const Server& 		getServer() const;
		void 				changeServer(Server server);
};

#endif
