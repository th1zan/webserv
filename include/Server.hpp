#ifndef SERVER_HPP
#define SERVER_HPP

#include "defines.hpp"
#include "Parser.hpp"
#include "utils.hpp"

typedef struct location_s
{
	std::string					root;
	std::vector<std::string>	methods;
	std::string					redirect;
	bool						autoindex;
	std::string					tryFile;
	bool						hasCGI;
	std::string					cgiPath;
	std::string					cgiExtension;
	std::string					uploadTo;
}					location_t;

class Server{
	private:
			std::string 		_serverName;
			std::string 		_port;
			std::string 		_host;
			std::string 		_root;
			std::string 		_index;
			size_t				_clientMaxBodySize;
			std::string 		_errorPage;
			std::string 		_errorResponse;
			std::vector<location_t>	_tempLocationVector;
			bool				_isDefault;
			int					_socket;


			


			//Servers
			// std::vector<Server>					_serversVector;
			std::map<std::string, std::string>	_ServerConfigMap;


			//Location
			std::vector<std::map<std::string, std::string>>		_LocationMapVector;
			std::map<std::string, std::string>					_LocationConfigMap;
			std::vector<Server::location_t>						_LocationVector;
			void				_getLocationStruct();

	public:
		Server():
		

		///verifier que les noms de servers ne sont pas les memes !

};

#endif