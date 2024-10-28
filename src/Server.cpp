#include "Server.hpp"

Server::Server(std::vector<Server>&	_serversVector, std::map<std::string, std::string> tempServerConfigMap, std::vector<std::map<std::string, std::string> > tempLocationMapVector) {

	this->_ServerConfigMap = tempServerConfigMap;
	this->_LocationMapVector = tempLocationMapVector;
	

	//server bloc param
	this->_serverName = this->_ServerConfigMap[SERVER_N];
	this->_host = this->_ServerConfigMap[HOST];
	this->_root = this->_ServerConfigMap[ROOT_LOC];
	this->_index = this->_ServerConfigMap[INDEX];
	this->_errorPage = this->_ServerConfigMap[ERROR_P];
	this->_port = this->_ServerConfigMap[LISTEN];
	this->_clientMaxBodySize = this->_getConvertedMaxSize(_ServerConfigMap[MAX_SIZE]);
	
	//get location bloc param to a vector of location_t struct
	
	if (!tempLocationMapVector.empty())
	{
		this->_getLocationStruct();
	}
	


	// ??? Error response
	// this->_errorResponse = this->_generateErrorResponse();


	this->_isDefault = this->_checkDefaultServer(_serversVector);
	
	//DEBUG
	std::cout << "Server constructor called" << std::endl;
}

long Server::_getConvertedMaxSize(std::string& maxSizeStr) {
	// Utiliser std::stoll pour convertir la chaîne en long
	try {
		long value = std::stoll(maxSizeStr);
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
	// DEBUG
	// std::cout << std::endl << "in: _getLocationStruct:: print _LocationMap (vector):" << std::endl;

	for (std::vector<std::map<std::string, std::string> >::iterator itMap = _LocationMapVector.begin(); itMap != _LocationMapVector.end(); ++itMap) {
		
		// DEBUG
		// std::cout << std::endl;
		// printMap(*itMap);

		location_t tmpLoc; // Initialise une structure temporaire

		// Assigner le champ root
		if (itMap->find("root") != itMap->end())
			tmpLoc.root = itMap->find("root")->second;

		// Assigner le champ methods
		if (itMap->find("allow_methods") != itMap->end()) {
			std::string methodsStr = itMap->find("allow_methods")->second;
			std::istringstream iss(methodsStr);
			std::string method;
			while (iss >> method) {
				tmpLoc.methods.push_back(method);
			}
		}

		// Assigner le champ redirect
		if (itMap->find("return") != itMap->end())
			tmpLoc.redirect = itMap->find("return")->second;

		// Assigner le champ autoindex
		if (itMap->find("autoindex") != itMap->end()) {
			tmpLoc.autoindex = (itMap->find("autoindex")->second == "on");
		}
		else
			tmpLoc.autoindex = false;

		// Assigner le champ tryFile
		if (itMap->find("try_file") != itMap->end())
			tmpLoc.tryFile = itMap->find("try_file")->second;

		// Assigner le champ hasCGI
		if (itMap->find("hasCGI") != itMap->end()) {
			tmpLoc.hasCGI = (itMap->find("hasCGI")->second == "true");
		}

		// Assigner le champ cgiPath
		if (itMap->find("cgi_path") != itMap->end())
			tmpLoc.cgiPath = itMap->find("cgi_path")->second;

		// Assigner le champ cgiExtension
		if (itMap->find("cgi_ext") != itMap->end())
			tmpLoc.cgiExtension = itMap->find("cgi_ext")->second;

		// Assigner le champ uploadTo
		if (itMap->find("upload_to") != itMap->end())
			tmpLoc.uploadTo = itMap->find("upload_to")->second;

		this->_LocationMap[itMap->find("location")->second]=(tmpLoc);

		// DEBUG
		// printLocation(tmpLoc);
	}
}




Server::~Server(){
	std::cout << "Server destructor called" <<std::endl;
}



/******* Public Functions called by Service during Setup() *******/


// ---> Getters ---------------------------------------------------------------
bool Server::getIsDefault(){return (this->_isDefault);}
const std::string&	Server::getHost() const{return this->_host;}
const std::string&	Server::getPort() const{return this->_port;}
int					Server::getSocket() const{return this->_socket;}
const std::string&	Server::getServerName() const{return this->_serverName;}
const std::string&	Server::getRoot() const{return this->_root;}
const std::string&	Server::getIndex() const{return this->_index;}
const std::string&	Server::getErrorPage() const{return this->_errorPage;}
const std::string&	Server::getErrorResponse() const{return this->_errorResponse;}
size_t				Server::getClientMaxBodySize() const{return this->_clientMaxBodySize;}

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



void Server::printServers() {
	
		std::cout << std::endl;
		std::cout << "Server Name: " << this->_serverName << std::endl;
		std::cout << "Host: " << this->_host << std::endl;
		std::cout << "Root: " << this->_root << std::endl;
		std::cout << "Index: " << this->_index << std::endl;
		std::cout << "Error Page: " << this->_errorPage << std::endl;
		std::cout << "Port: " << this->_port << std::endl;
		std::cout << "Client Max Body Size: " << this->_clientMaxBodySize << std::endl;
		std::cout << "Is Default: " << (this->_isDefault ? "Yes" : "No") << std::endl;
		
		// Loop through each location in the server
		for (std::map<std::string, location_t>::const_iterator locIt = this->_LocationMap.begin(); locIt != this->_LocationMap.end(); ++locIt) {
			std::cout << std::endl;
			location_t tmploc = locIt->second;
			std::cout << "  Location Name: " << locIt->first << std::endl;
			std::cout << "  Location Root: " << tmploc.root << std::endl;
			std::cout << "  Redirect: " << tmploc.redirect << std::endl;
			std::cout << "  Autoindex: " << (tmploc.autoindex ? "Enabled" : "Disabled") << std::endl;
			std::cout << "  Try File: " << tmploc.tryFile << std::endl;
			std::cout << "  Has CGI: " << (tmploc.hasCGI ? "Yes" : "No") << std::endl;
			std::cout << "  CGI Path: " << tmploc.cgiPath << std::endl;
			std::cout << "  CGI Extension: " << tmploc.cgiExtension << std::endl;
			std::cout << "  Upload To: " << tmploc.uploadTo << std::endl;

			std::cout << "  Methods: ";
			for (std::vector<std::string>::const_iterator methodIt = tmploc.methods.begin(); methodIt != tmploc.methods.end(); ++methodIt) {
				std::cout << *methodIt << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "----------------------------------------" << std::endl;
}

void Server::printLocation(location_t loc) {
	
		std::cout << std::endl;
		std::cout << "Print Location: " << std::endl;
			std::cout << std::endl;
			std::cout << "  Location Root: " << loc.root << std::endl;
			std::cout << "  Redirect: " << loc.redirect << std::endl;
			std::cout << "  Autoindex: " << (loc.autoindex ? "On" : "Off") << std::endl;
			std::cout << "  Try File: " << loc.tryFile << std::endl;
			std::cout << "  Has CGI: " << (loc.hasCGI ? "Yes" : "No") << std::endl;
			std::cout << "  CGI Path: " << loc.cgiPath << std::endl;
			std::cout << "  CGI Extension: " << loc.cgiExtension << std::endl;
			std::cout << "  Upload To: " << loc.uploadTo << std::endl;

			std::cout << "  Methods: ";
			for (std::vector<std::string>::const_iterator methodIt = loc.methods.begin(); methodIt != loc.methods.end(); ++methodIt) {
				std::cout << *methodIt << " ";
			}
			std::cout << std::endl;
}
