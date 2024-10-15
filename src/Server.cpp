#include "Server.hpp"

Server::Server(std::map<std::string, std::string> tempServerConfigMap, std::vector<std::map<std::string, std::string>> tempLocationMapVector) {

	this->_ServerConfigMap = tempServerConfigMap;
	this->_LocationMapVector = tempLocationMapVector;
	

	//server bloc param
	this->_serverName = this->_ServerConfigMap[SERVER_N];
	this->_host = this->_ServerConfigMap[HOST];
	this->_root = this->_ServerConfigMap[ROOT]);
	this->_index = this->_ServerConfigMap[INDEX];
	this->_errorPage = this->_ServerConfigMap[ERROR_P];
	this->_port = this->_ServerConfigMap[LISTEN];
	this->_clientMaxBodySize = this->_getConvertedMaxSize(configs[MAX_SIZE]);
	
	//get location bloc param to a vector of location_t struct
	this->_getLocationStruct();

	// ??? Error response
	// this->_errorResponse = this->_generateErrorResponse();


	//if the server is made with default parameters because there is no config file as argument, the default config file is loaded
	// this->_isDefault = 
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