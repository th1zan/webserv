#include "Server.hpp"

Server::Server(std::vector<Server>&	_serversVector, std::map<std::string, std::string> tempServerConfigMap, std::vector<std::map<std::string, std::string>> tempLocationMapVector) {

	this->_ServerConfigMap = tempServerConfigMap;
	this->_LocationMapVector = tempLocationMapVector;
	

	//server bloc param
	this->_serverName = this->_ServerConfigMap[SERVER_N];
	this->_host = this->_ServerConfigMap[HOST];
	this->_root = this->_ServerConfigMap[ROOT];
	this->_index = this->_ServerConfigMap[INDEX];
	this->_errorPage = this->_ServerConfigMap[ERROR_P];
	this->_port = this->_ServerConfigMap[LISTEN];
	this->_clientMaxBodySize = this->_getConvertedMaxSize(_ServerConfigMap[MAX_SIZE]);
	
	//get location bloc param to a vector of location_t struct
	this->_getLocationStruct();

	// ??? Error response
	// this->_errorResponse = this->_generateErrorResponse();


	this->_isDefault = this->_checkDefaultServer(_serversVector);
}

long long _getConvertedMaxSize(const std::string& maxSizeStr) {
	// Utiliser std::stoll pour convertir la chaîne en long long
	try {
		long long value = std::stoll(maxSizeStr);
		return value;
	} catch (const std::invalid_argument& e) {
		throw std::runtime_error(ERR_MAX_SIZE_CONVERSION(maxSizeStr));
	} catch (const std::out_of_range& e) {
		throw std::runtime_error(ERR_MAX_SIZE_CONVERSION_LONG(maxSizeStr));
	}
}

/*
Location map keywords:
	LOCATION
	ALLOW_M
	TRY
	RETURN
	AUTOID
	ROOT_LOC
	UPLOAD
	CGI_P
	CGI_E
*/

bool Server::_checkDefaultServer(std::vector<Server>&	_serversVector)
{
	std::vector<Server>::const_iterator previous = _serversVector.begin();
	for (; previous != _serversVector.end(); previous++)
	{
		if (previous->_host == this->_host && previous->_port == this->_port)
			return false;
	}
	return true;
}

void Server::_getLocationStruct() {
	// Iterate through each map in the _LocationMapVector
	for (std::vector<std::map<std::string, std::string>>::iterator it = _LocationMapVector.begin(); it != _LocationMapVector.end(); ++it) {
		location_t location;  // Initialize a location_t struct

		// Get the current map from the vector
		std::map<std::string, std::string>& configMap = *it;

		// Fill the location_t fields with values from the map

		// Check if the 'root' key exists and assign its value to the struct
		if (configMap.find("root") != configMap.end())
			location.root = configMap["root"];

		// Check if the 'methods' key exists and split the methods into a vector
		if (configMap.find("methods") != configMap.end()) {
			std::string methodsStr = configMap["methods"];
			// Split the methods string (e.g., "GET POST") and store them in the methods vector
			std::istringstream iss(methodsStr);
			std::string method;
			while (iss >> method) {
				location.methods.push_back(method);
			}
		}

		// Assign the 'redirect' value if it exists
		if (configMap.find("redirect") != configMap.end())
			location.redirect = configMap["redirect"];

		// Convert the 'autoindex' string value to a boolean (e.g., "on" -> true)
		if (configMap.find("autoindex") != configMap.end()) {
			location.autoindex = (configMap["autoindex"] == "on");
		}

		// Assign the 'try_files' value if it exists
		if (configMap.find("try_files") != configMap.end())
			location.tryFile = configMap["try_files"];

		// Convert 'hasCGI' from a string to a boolean ("true" -> true)
		if (configMap.find("hasCGI") != configMap.end()) {
			location.hasCGI = (configMap["hasCGI"] == "true");
		}

		// Assign the 'cgi_path' value if it exists
		if (configMap.find("cgi_path") != configMap.end())
			location.cgiPath = configMap["cgi_path"];

		// Assign the 'cgi_extension' value if it exists
		if (configMap.find("cgi_extension") != configMap.end())
			location.cgiExtension = configMap["cgi_extension"];

		// Assign the 'upload_to' value if it exists
		if (configMap.find("upload_to") != configMap.end())
			location.uploadTo = configMap["upload_to"];

		// Add the filled location struct to the _LocationVector
		_LocationVector.push_back(location);
	}
}



Server::~Server(){}

/******* Public Functions called by Service during Setup() *******/


// ---> Getters ---------------------------------------------------------------
bool Server::getIsDefault(){return (this->_isDefault);}
const std::string&	Server::getHost() const{return this->_host;}
const std::string&	Server::getPort() const{return this->_port;}
int					Server::getSocket() const{return this->_socket;}
const std::string&	Server::getServerName() const{return this->_serverName;}

// std::string const	&Server::getHost() const{return this->_host;}

// std::string const	&Server::getRoot() const{return this->_root;}
// std::string const	&Server::getIndex() const{return this->_index;}
// std::string const	&Server::getErrorPage() const{return this->_errorPage;}
// std::string const	&Server::getErrorResponse() const{return this->_errorResponse;}
// size_t				Server::getClientMaxBodySize() const{return this->_clientMaxBodySize;}

// locationMap const	&Server::getLocations() const{return this->_locations;}

void Server::createSocket()
{
	if (!this->_socket)
	{
		this->_socket = socket(AF_INET, SOCK_STREAM, 0);

		if (this->_socket < 0)
			throw std::runtime_error(ERR_SOCKET(this->_serverName));
	}
}

// void Server::addLocation(locationPair location)
// {
// 	this->_locations.insert(location);
// }