#ifndef SERVER_HPP
#define SERVER_HPP

#include "defines.hpp"
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
} location_t;

class Server{
	private:
			std::string 					_serverName;
			std::string 					_port;
			std::string 					_host;
			std::string 					_root;
			std::string 					_index;
			long							_clientMaxBodySize;
			std::string 					_errorPage;
			std::string 					_errorResponse;
			std::vector<location_t>			_tempLocationVector;
			bool							_isDefault;
			int								_socket;

			bool							_checkDefaultServer(std::vector<Server>& _serversVector);
			long 							_getConvertedMaxSize(std::string& maxSizeStr);
			


			//Servers
			// std::vector<Server>					_serversVector;
			std::map<std::string, std::string>					_ServerConfigMap;


			//Location
			std::vector<std::map<std::string, std::string> >		_LocationMapVector;
			std::map<std::string, std::string>					_LocationConfigMap;
			std::map<std::string, location_t>					_LocationMap;
			void												_getLocationStruct();

	public:
			Server(std::vector<Server>&	_serversVector, std::map<std::string, std::string> tempServerConfigMap, std::vector<std::map<std::string, std::string> > tempLocationMapVector);
			~Server();
			//called from Service
			void	createSocket();

			//getters
			bool				getIsDefault();
			const std::string& 	getHost() const;
			const std::string& 	getPort() const;
			int					getSocket() const;
			const std::string& 	getServerName() const;
			const std::string&	getRoot() const;
			const std::string&	getIndex() const;
			const std::string&	getErrorPage() const;
			const std::string&	getErrorResponse() const;
			size_t				getClientMaxBodySize() const;



			//utils
			void 				printServers();
			void 				printLocation(location_t loc);



		///verifier que les noms de servers ne sont pas les memes !

};

#endif
