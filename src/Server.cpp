/**
 * @file Server.cpp
 * @brief Implementation of the Parser class for processing the 'server' blocs and 'location' sub-blocs in the configuration file in order to instanciate the server classes.
 * 
 */

#include "Server.hpp"
#include <sstream>

/**
 * @brief Constructs a `Server` object with the parsed configuration parameters.
 * 
 * The configuration data consists of a map for server settings and a vector of maps for location-specific settings.
 * These parameters are used to set up various attributes of the server, 
 * such as host, root directory, index file, and more.
 * 
 * 
 * @details The constructor performs the following operations:
 * - Uploads raw parameters from the provided maps (Server bloc parameters) and vector (multiple Location bloc parameters).
 * - Extracts and assigns server attributes such as server name, host, root, index, and port.
 * - Initializes the server socket to zero.
 * - Determines if this server instance is the primary one by checking other instances in `_serversVector`.
 */
Server::Server(std::vector<Server>&	_serversVector, std::map<std::string, std::string> tempServerConfigMap, std::vector<std::map<std::string, std::string> > tempLocationMapVector) {

	//upload the "raw" parameters from 1 maps corresponding to the server bloc Server and 1 vector of map corresponding to the Location blocs.
	this->_ServerConfigMap = tempServerConfigMap;
	this->_LocationMapVector = tempLocationMapVector;
	
	//extract the parameters from maps
	this->_fillServerNameVector(_ServerConfigMap[SERVER_N]);
	this->_host = this->_ServerConfigMap[HOST];
	this->_root = this->_ServerConfigMap[ROOT_LOC];
	this->_index = this->_ServerConfigMap[INDEX];
	this->_fillErrorPageMap();
	this->_port = this->_ServerConfigMap[LISTEN];
	this->_clientMaxBodySize = this->_getConvertedMaxSize(_ServerConfigMap[MAX_SIZE]);
	
	//if any error_page directive, create one
	// this->_errorResponse = this->_generateErrorResponse();

	//initialisation
	this->_socket = 0;
	
	//get location bloc parameters from to `tempLocationMapVector` to a vector of struct location_t 
	if (!tempLocationMapVector.empty())
	{
		this->_getLocationStruct();
	}

	//check if there is other instance of server with the same `_host` `_port`
	this->_isPrimary = this->_checkPrimaryServer(_serversVector);
	
	//DEBUG log 
	this->printServers();
}

/**
 * The Server class destructor.
 */
Server::~Server(){
}

/**
 * The function `_fillServerNameVector` reads a string of server names, splits it into individual
 * words, and stores them in a vector.
 *
 */
void Server::_fillServerNameVector(std::string& serverNames){
	std::istringstream iss(serverNames);
	std::string word;

	while (iss >> word) {
		_serverNameVector.push_back(word);
	}
}

/**
 * The function `_fillErrorPageMap` iterates through a map, extracts error numbers and corresponding
 * error pages name, and populates another map with these pairs (errorNumber <-> errorPage).
 */
void Server::_fillErrorPageMap(){
	for (std::map<std::string, std::string>::iterator it = _ServerConfigMap.begin(); it != _ServerConfigMap.end(); ++it) {
		if (it->first.find(ERROR_P) == 0) {
			std::string errorPageString(it->second);
			std::istringstream iss(errorPageString);
			std::string tmpErrorNb;
			std::string tmpErrorPage;

			// Extract all values into a vector
			std::vector<std::string> tmpVec;
			while (iss >> tmpErrorNb)
			{
				tmpVec.push_back(tmpErrorNb);
			}
			tmpErrorPage = tmpVec.back();

			// Push all errorNb-errorPage pairs in the map
			for (size_t i = 0; i < tmpVec.size() - 1; ++i) {
				this->_errorPages[tmpVec[i]] = tmpErrorPage;
			}
		}
	}
}

/**
 * The function `_getConvertedMaxSize` converts a string to a long using `std::stoll` and
 * handles potential exceptions.
 * 
 * @param maxSizeStr The `maxSizeStr` parameter is a `std::string` that represents the maximum size
 * value that needs to be converted to a `long` integer. The `_getConvertedMaxSize` function attempts
 * to convert this string value to a `long` using `std::stoll` and handles
 * 
 * @return The function `_getConvertedMaxSize` is returning a `long` value after converting the
 * `maxSizeStr` string parameter to a long using `std::stoll`.
 */
long Server::_getConvertedMaxSize(std::string& maxSizeStr) {
	try {
		long value = std::stoll(maxSizeStr);
		return value;
	} catch (const std::invalid_argument& e) {
		throw std::runtime_error(ERR_MAX_SIZE_CONVERSION(maxSizeStr));
	} catch (const std::out_of_range& e) {
		throw std::runtime_error(ERR_MAX_SIZE_CONVERSION_LONG(maxSizeStr));
	}
}

/**
 * @brief The function checks if there is an other server block with the same Host and Port.
 * 
 * @param _serversVector 
 * @return true if the server is 'unique' 
 * @return false 
 */
bool Server::_checkPrimaryServer(std::vector<Server>&	_serversVector)
{
	std::vector<Server>::const_iterator previous = _serversVector.begin();
	for (; previous != _serversVector.end(); previous++)
	{
		if (previous->_host == this->_host && previous->_port == this->_port)
			return false;
	}
	return true;
}

/**
 * The function `_getLocationStruct` iterates through a vector of maps to extract and store
 * location-related data into a custom data structure.
 */
void Server::_getLocationStruct() {
	for (std::vector<std::map<std::string, std::string> >::iterator itMap = _LocationMapVector.begin(); itMap != _LocationMapVector.end(); ++itMap) {		
		location_t tmpLoc;

		if (itMap->find("root") != itMap->end())
			tmpLoc.root = itMap->find("root")->second;

		if (itMap->find("allow_methods") != itMap->end()) {
			std::string methodsStr = itMap->find("allow_methods")->second;
			std::istringstream iss(methodsStr);
			std::string method;
			while (iss >> method) {
				tmpLoc.methods.push_back(method);
			}
		}

    if (itMap->find("return") != itMap->end()) {
      std::istringstream tmp(itMap->find("return")->second);
      std::vector<std::string> tokens;
      std::string word;

      while (tmp >> word) {
          tokens.push_back(word);
      }

      if (tokens.size() == 2) {
          tmpLoc.redirect_err = tokens[0];
          tmpLoc.redirect_path = tokens[1];
      }
      else if (tokens.size() == 1) {
          tmpLoc.redirect_path = tokens[0];
      }
      else {
        std::cerr << "Error in "return" directive" << std::endl;
      }
    }


    }			

		if (itMap->find("autoindex") != itMap->end()) {
			tmpLoc.autoindex = (itMap->find("autoindex")->second == "on");
		}
		else
			tmpLoc.autoindex = false;

		if (itMap->find("try_file") != itMap->end())
			tmpLoc.tryFile = itMap->find("try_file")->second;

		if (itMap->find(HAS_CGI) != itMap->end()) {
			tmpLoc.hasCGI = (itMap->find(HAS_CGI)->second == "true");
			// Debug to ensure hasCGI is parsed correctly (Jannetta) - to remove later
			std::cout << "Assigning hasCgi: " << tmpLoc.hasCGI << " for location " << itMap->find("location")->second << std::endl;
			std::cout << "Location: " << itMap->find("location")->second
			<< ", Has CGI: " << tmpLoc.hasCGI
			<< ", CGI Path: " << tmpLoc.cgiPath
			<< ", CGI Extension: " << tmpLoc.cgiExtension << std::endl;
		}

		if (itMap->find("cgi_path") != itMap->end())
			tmpLoc.cgiPath = itMap->find("cgi_path")->second;

		if (itMap->find("cgi_ext") != itMap->end())
			tmpLoc.cgiExtension = itMap->find("cgi_ext")->second;

		if (itMap->find("upload_to") != itMap->end())
		{
			tmpLoc.uploadTo = itMap->find("upload_to")->second;
			// Debug to ensure upload_to is parsed correctly
    		std::cout << "Parsed upload_to: " << tmpLoc.uploadTo << " for location: " << itMap->find("location")->second << std::endl;
		}

        this->_LocationMap[itMap->find("location")->second] = tmpLoc;
		this->_LocationMap[itMap->find("location")->second]=(tmpLoc);
	}
}

// ---> Getters ---------------------------------------------------------------
bool                      Server::getIsPrimary(){return (this->_isPrimary);}
const std::string&	      Server::getHost() const{return this->_host;}
const std::string&	      Server::getPort() const{return this->_port;}
int							          Server::getSocket() const{return this->_socket;}
std::vector<std::string>	Server::getServerNameVector() const{return this->_serverNameVector;}
size_t						        Server::getClientMaxBodySize() const{return this->_clientMaxBodySize;}
const std::string&			  Server::getRoot() const{return this->_root;}
const std::string&			  Server::getIndex() const{return this->_index;}
std::map<std::string, std::string>			Server::getErrorPage() const{return this->_errorPages;}
const std::string&			                Server::getErrorResponse() const{return this->_errorResponse;}
// locationMap const	&Server::getLocations() const{return this->_locations;}

// Jannetta - added getter for location map
const std::map<std::string, location_t>& Server::getLocations() const{return this->_LocationMap;}

location_t Server::getLocationConfig(const std::string &path) const
{
    const std::map<std::string, location_t> &locations = getLocations();
    const location_t *matchedLocation = NULL;
    size_t matchedPrefixLength = 0;

    for (std::map<std::string, location_t>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
        if (path.find(it->first) == 0 && it->first.length() > matchedPrefixLength)
		{
            matchedLocation = &(it->second);
            matchedPrefixLength = it->first.length();
        }
    }

    if (matchedLocation)
	{
        // Debugging to ensure the correct location is matched
        std::cout << "Matched location: " << matchedPrefixLength << ", uploadTo: " << matchedLocation->uploadTo << std::endl;
        return *matchedLocation;
    }

    throw std::runtime_error("No matching location found for path: " + path);
}

void Server::createSocket()
{
	if (!this->_socket)
	{
		this->_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (this->_socket < 0)
			throw std::runtime_error(ERR_SOCKET(this->_serverNameVector.at(0)));
	}
}
/**
 * The function `printServers` in the `Server` class prints detailed information about server
 * configurations and locations.
 */
void Server::printServers() {
	
		std::cout << std::endl;
		std::cout << "----- Server::printServers()----" << std::endl;
		std::cout << "Server Name: " ;
		for (std::vector<std::string>::const_iterator it = _serverNameVector.begin(); it != _serverNameVector.end(); ++it) {
			std::cout << *it << " ";
			}
		std::cout << std::endl;
		std::cout << "Host: " << this->_host << std::endl;
		std::cout << "Root: " << this->_root << std::endl;
		std::cout << "Index: " << this->_index << std::endl;
		std::cout << "Error Page: " << std::endl;
		for (std::map<std::string, std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); ++it) {
			std::cout << "\t"<< it->first << " " << it->second << std::endl;
		}	
		std::cout << "Port: " << this->_port << std::endl;
		std::cout << "Client Max Body Size: " << this->_clientMaxBodySize << std::endl;
		std::cout << "Is Primary: " << (this->_isPrimary ? "Yes" : "No") << std::endl;
		
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
			
			// Jannetta's DEBUG - to delete later
			// Debug for location information
        	std::cout << "Location Name: " << locIt->first
                  << ", Has CGI: " << (tmploc.hasCGI ? "Yes" : "No")
                  << ", CGI Path: " << tmploc.cgiPath
                  << ", CGI Extension: " << tmploc.cgiExtension << std::endl;


			std::cout << "  Methods: ";
			for (std::vector<std::string>::const_iterator methodIt = tmploc.methods.begin(); methodIt != tmploc.methods.end(); ++methodIt) {
				std::cout << *methodIt << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "----- /END Server::printServers() ----" << std::endl;
		std::cout << std::endl;
}

/**
 * The function `Server::printLocation` prints information about a location configuration in a server.
 * 
 */
void Server::printLocation(location_t loc) {
	
		std::cout << std::endl;
		std::cout << "----- Server::printLocation() ----" << std::endl;
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
			std::cout << "----- /END Server::printLocation() ----" << std::endl;
			std::cout << std::endl;
}

