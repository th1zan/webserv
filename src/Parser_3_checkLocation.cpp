/**
 * @file Parser_3_checkLocation.cpp
 * @brief This file containes the functions used to check the Location parameters in each 'Server'.
 */

#include "Parser.hpp"

void	Parser::_checkLocationParam(){

	this->_checkLocDirName();
	this->_checkLocDirValue();
}

void	Parser::_checkLocDirName(){
	
	for(std::vector<std::map<std::string, std::string> >::const_iterator itVec = this->_tempLocationMapVector.begin(); itVec != this->_tempLocationMapVector.end(); itVec++){
		_tempLocationConfigMap = *itVec;
		//DEBUG
		// std::cout << "in :: '_checkLocDirName'" << std::endl;
		// printMap(*it);

		//No need to control location
		// std::string const mandatoryParam[] = {LOCATION};

		//forbiden = Location parameter
		std::string const forbiddenParam[] = {LISTEN, HOST, ROOT_LOC, INDEX, MAX_SIZE, SERVER_N, ERROR_P};

		// for (int i = 0; i < 1; i++)
		// {
		// 	if (_tempLocationConfigMap.find(mandatoryParam[i]) == _tempLocationConfigMap.end())
		// 		throw std::runtime_error(ERR_SERV_DIRECTIVE_MISSING(mandatoryParam[i]));
		// }
		
		for (int i = 0; i < 7; i++)
		{
			if (this->_tempLocationConfigMap.find(forbiddenParam[i]) != this->_tempLocationConfigMap.end())
				throw std::runtime_error(ERR_LOC_FORBIDDEN_DIRECTIVE(forbiddenParam[i]));
		}
	}
}


void	Parser::_checkLocDirValue(){
	//DEBUG
	// std::cout << "in _checkLocDirValue::" << std::endl;
	

	for(std::vector<std::map<std::string, std::string> >::iterator itVec = this->_tempLocationMapVector.begin(); itVec != this->_tempLocationMapVector.end(); itVec++){

		//DEBUG
		// std::cout << "New Location" << std::endl;
		// printMap(*itVec);
		
		for (std::map<std::string, std::string>::iterator itMap = itVec->begin(); itMap != itVec->end(); ++itMap) {
			//DEBUG
			// std::cout << "itMap->first:" << itMap->first << std::endl;
			
			if (itMap->first == LOCATION) {
				this->_checkLocation(itMap->second);
			}
			else if (itMap->first == ALLOW_M) {
				this->_checkAllowM(itMap->second);
			}
			else if (itMap->first == TRY) {
				this->_checkTry(itMap->second);
			}
			else if (itMap->first == RETURN) {
				this->_checkReturn(itMap->second);
			}
			else if (itMap->first == AUTOID) {
				this->_checkAutoID(itMap->second);
			}
			else if (itMap->first == ROOT_LOC) {
				this->_checkRootLoc(itMap->second);
			}
			else if (itMap->first == UPLOAD) {
				this->_checkUpload(itMap->second);
			}
			else if (itMap->first == CGI_P) {
				this->_checkCgiP(itMap->second);
			}
			else if (itMap->first == CGI_E) {
				this->_checkCgiE(itMap->second);
			}

			//DEBUG
			// std::cout << " in _checkLocDirValue:: itMap->first:" << itMap->first << ", itMap->second: " << itMap->second << std::endl;
		}
	}

}

void Parser::_checkPath(std::string& path, bool isDir, bool hasWPerm) {
	struct stat info;

	std::string fullPath = this->_tempRootDirPath + path;

	// Check for existence
	if (access(fullPath.c_str(), F_OK) != 0) {
		throw std::runtime_error(ERR_DIRECTORY(fullPath));
	}

	// throw std::runtime_error(ERR_DIRECTORY(dirValue));

	// Get file information
	if (stat(fullPath.c_str(), &info) != 0) {
		throw std::runtime_error(ERR_FILE_INFO(fullPath));
	}

	// Check if it's a directory or a file
	if ((info.st_mode & S_IFDIR) != 0) { // It's a directory
		if (!isDir) {
			throw std::runtime_error(ERR_DIRECTORY(fullPath));
		}
	} else if ((info.st_mode & S_IFREG) != 0) { // It's a regular file
		if (isDir) {
			throw std::runtime_error(ERR_FILE(fullPath)); 
		}
	} else {
		throw std::runtime_error(ERR_NOT_FILE_NOT_DIR(fullPath));
	}
	
	// Check for write access
	// Check for write access if hasWPerm is true
	if (hasWPerm && access(fullPath.c_str(), W_OK) != 0) {
		throw std::runtime_error(ERR_PATH(fullPath));
	}
}


//------ Directives' Location checking functions:
void Parser::_checkLocation(std::string& dirValue) {
		//DEBUG
		// std::cout << "in: _checkLocation:: dirValue = " << dirValue << std::endl;

		
		/*
		The value after `location` determines which segment of a URL or which type of file the block applies to.
		 It can be a specific path, a file extension, or a regex pattern.
		*/

		//ANY verification for now, only delete the ending '{'

		 while (!dirValue.empty() && (std::isspace(dirValue.back()) || dirValue.back() == '{')) {
			dirValue.erase(dirValue.size() - 1);
		 }
		// if (dirValue.empty() || dirValue[0] != '/') {
		// 	throw std::runtime_error(ERR_LOCATION(dirValue));
		// }
}

void Parser::_checkAllowM(std::string& dirValue) {
	// std::cout << " in _checkAllowM:: BEFORE dirValue:" << dirValue << std::endl;
	// delete ending ';' if necessary to get a cleaner string later
		this->_delEndSemiColon(dirValue);

	// std::cout << " in _checkAllowM:: AFTER dirValue:" << dirValue <<std::endl;

	// Set of allowed methods
	std::set<std::string> allowedMethods;
	allowedMethods.insert("GET");
	allowedMethods.insert("POST");
	allowedMethods.insert("DELETE");

	// Split the methods by space and check validity
	std::stringstream ss(dirValue);
	std::string method;
	while (ss >> method) {
		if (allowedMethods.find(method) == allowedMethods.end()) {
			throw std::runtime_error(ERR_INVALID_METHOD(method, dirValue));
		}
	}

}

void Parser::_checkTry(std::string& dirValue) {

	/*function seems identical to _checkIndex !!! probably possible to use it*/ 

	// delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	//OLD FASHIONED
	// find 'root' path in the map
	// std::map<std::string, std::string>::const_iterator it = _tempServerConfigMap.find(ROOT_LOC);
	// if (it == _tempServerConfigMap.end()) {
	// 	throw std::runtime_error(ERR_SERV_DIRECTIVE_MISSING(dirValue));
	// }
	// // Get path
	// std::string rootDir = it->second; 
	// this->_delEndSemiColon(rootDir);

	// make fullPath to the file
	std::string fullPath = this->_tempRootDirPath + "/" + dirValue;

	// Vérifier si le fichier existe
	int fd = open(fullPath.c_str(), O_RDONLY);
	if (fd < 0) {
		throw std::runtime_error(ERR_FILE(fullPath));
	}
	// Fermer le descripteur de fichier si l'ouverture a réussi
	close(fd);
}


void Parser::_checkReturn(std::string& dirValue) {

	/*function seems identical to _checkRoot !!! probably possible to use it*/ 

	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);
	this->_checkPath(dirValue, false);
}



void Parser::_checkAutoID(std::string& dirValue) {
	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	std::transform(dirValue.begin(), dirValue.end(), dirValue.begin(), ::tolower);
	if (dirValue != "on" && dirValue != "off")
		throw std::runtime_error(ERR_AUTOINDEX);
	
	//DEBUG
	// std::cout << "in _checkAutoID:: dirValue = " << dirValue << std::endl;
}


void Parser::_checkRootLoc(std::string& dirValue) {
	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);
	this->_checkPath(dirValue, true);
}

void Parser::_checkUpload(std::string& dirValue) {
	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);
	this->_checkPath(dirValue, true, true);
}


void Parser::_checkCgiP(std::string& dirValue) {
	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	// check directory
	this->_checkPath(dirValue, true);

	std::string fullPath = this->_tempRootDirPath + dirValue;
	//  open directory
	DIR* dir = opendir(fullPath.c_str());
	if (!dir) {
		throw std::runtime_error(ERR_OPEN_DIR(dirValue));
	}

	struct dirent* entry;
	// bool hasCgi = false; //not used for now

	// loop to list the files in the directory and check CGI
	while ((entry = readdir(dir)) != nullptr) {
		// ignore the special files '.' and '..'
		if (entry->d_name[0] == '.') {
			continue;
		}

		// get the filePath
		std::string filePath = fullPath + "/" + entry->d_name;
		struct stat fileInfo;

		// Get info
		if (stat(filePath.c_str(), &fileInfo) == 0) {
			// check if the file is regular (not a directory and not a link/alias) AND executable
			if ((fileInfo.st_mode & S_IFREG) && (fileInfo.st_mode & S_IXUSR)) {
				this->_tempLocationConfigMap["hasCgi"] = "true";
				break;
			}
		}
	}
	closedir(dir);

}

void Parser::_checkCgiE(std::string& dirValue) {
	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	//list of supported extension (created in a dedicated function 'getSupportedExtensions')
	// const std::set<std::string> supportedExtensions = {".cgi", ".pl", ".py", ".sh", ".php"}; //initilisation list not possible in CPP98
	const std::set<std::string>& supportedExtensions = getSupportedExtensions();


	//check if there is a '.'
	if (dirValue.empty() || dirValue.front() != '.') {
		throw std::runtime_error("Extension must start with a point (.)");
	}

	// //check if the extension is supported (in the list)
	if (supportedExtensions.find(dirValue) == supportedExtensions.end()) {
		throw std::runtime_error("Unsupported CGI extension: " + dirValue);
	}
}


std::set<std::string> Parser::getSupportedExtensions() {
	std::set<std::string> extensions;
	extensions.insert(".cgi");
	extensions.insert(".pl");
	extensions.insert(".py");
	extensions.insert(".sh");
	extensions.insert(".php");
	return extensions;
}




/*******************************************/
	// Functions probably not needed
	//- isValidUrl
	//- urlExists
/*******************************************/

/*
bool isValidUrl(const std::string& url) {
	// Check if the URL starts with "http://" or "https://"
	if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://") {
		return false;
	}

	size_t pos = url.find("://");
	if (pos == std::string::npos) {
		return false; // "://" not found
	}

	// Check for a valid domain (simplified, allows letters, digits, dots, and hyphens)
	size_t start = pos + 3; // Move past "://"
	size_t end = url.find('/', start);
	if (end == std::string::npos) {
		end = url.length(); // No path found, use end of string
	}

	std::string domain = url.substr(start, end - start);
	for (size_t i = 0; i < domain.length(); ++i) {
		char c = domain[i];
		if (!std::isalnum(c) && c != '.' && c != '-') {
			return false; // Invalid character in domain
		}
	}

	// Check if an optional port is present
	size_t port_pos = domain.find(':');
	if (port_pos != std::string::npos) {
		std::string port = domain.substr(port_pos + 1);
		for (size_t i = 0; i < port.length(); ++i) {
			if (!std::isdigit(port[i])) {
				return false; // Port contains non-digit characters
			}
		}
	}

	return true; // URL is valid
}

#include <curl/curl.h>
bool urlExists(const std::string& url) {
	CURL* curl = curl_easy_init();
	if (!curl) return false;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // don't need the body of the response
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Timeout 10 sec

	// delete request
	CURLcode res = curl_easy_perform(curl);

	// check the answer
	long responseCode;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	
	curl_easy_cleanup(curl);
	
	// check if answer is 200 (OK)
	return (res == CURLE_OK && responseCode == 200);
}
*/



