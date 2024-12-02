/**
 * @file Server.hpp
 * @brief Header file for the Server class and location structure.
 *
 * This file contains the declaration of the Server class, which manages
 * server configurations and handles location directives.
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "defines.hpp"
#include "utils.hpp"

/**
 * @struct location_s
 * @brief Represents a location configuration for the server.
 *
 * This structure holds the configuration for a specific location, including
 * root path, allowed methods, redirect rules, autoindex settings, and
 * CGI configurations.
 */
typedef struct location_s {
	std::string			      		root;
	std::vector<std::string>	methods;
	std::string					      redirect_err;
	std::string					      redirect_path;
	bool						          autoindex;
	std::string					      tryFile;
	bool						          hasCGI;
	std::string					      cgiPath;
	std::string					      cgiExtension; 
	std::string					      uploadTo; 
} location_t;


/**
 * @class Server
 * @brief Manages server configurations and location directives.
 *
 * The Server class encapsulates server settings and provides methods
 * for handling server operations and location configurations.
 */
class Server{
	private:
			std::vector<std::string>			_serverNameVector;
			std::string 						      _port;
			std::string 						      _host;
			std::string 						      _root;
			std::string 						      _index;
			long								          _clientMaxBodySize;
			std::string 						      _errorResponse;
			std::vector<location_t>				_tempLocationVector;
			bool								          _isPrimary;
			int									          _socket;
			std::map<std::string, std::string>	_errorPages;

      bool								_checkPrimaryServer(std::vector<Server>& _serversVector);
			long 								_getConvertedMaxSize(std::string& maxSizeStr);
			void								_fillServerNameVector(std::string& serverNames);
			void								_fillErrorPageMap();

			// std::vector<Server>								_serversVector;
			std::map<std::string, std::string>					_ServerConfigMap;


			//Location
			std::vector<std::map<std::string, std::string> >	_LocationMapVector; //for parsing
			std::map<std::string, location_t>					_LocationMap; //to use
			void												_getLocationStruct();

	public:
			Server(std::vector<Server>&	_serversVector, std::map<std::string, std::string> tempServerConfigMap, std::vector<std::map<std::string, std::string> > tempLocationMapVector);
			~Server();
			void	createSocket();

			//getters
			bool							getIsPrimary();
			const std::string& 				getHost() const;
			const std::string& 				getPort() const;
			int								getSocket() const;
			std::vector<std::string>		getServerNameVector() const;
			const std::string&				getRoot() const;
			const std::string&				getIndex() const;
			std::map<std::string, std::string>				getErrorPage() const;
			const std::string&				getErrorResponse() const;
			size_t							getClientMaxBodySize() const;
			const std::map<std::string, location_t>& getLocations() const;  // Jannetta's function
			location_t getLocationConfig(const std::string &path) const; // Jannetta's function
			
			//utils
			void 							printServers();
			void 							printLocation(location_t loc);

};

#endif
