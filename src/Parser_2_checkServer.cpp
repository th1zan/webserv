/**
 * @file Parser_2_checkServer.cpp
 * @brief This file containes the functions used to check the 'Server' parameters except for its Location parameters.
 */

#include "Parser.hpp"


void Parser::_checkConfigs(){
	this->_checkServerParam();
	this->_checkLocationParam();

	//DEBUG
	// std::cout << "in :: '_checkConfigs': print _tempServerConfigMap" << std::endl;
	// printMap(this->_tempServerConfigMap);
	// printMap(*this->_tempLocationMapVector.begin());
	
}


void	Parser::_checkServerParam(){

	this->_checkDirectiveName();
	this->_checkDirectiveValue();

}


/**
 * @brief 
 * 
 */
void	Parser::_checkDirectiveName(){
	//DEBUG
	// std::cout << "in :: '_checkDirectiveName'" << std::endl;

	std::string const mandatoryParam[] = {LISTEN, HOST, ROOT_LOC, INDEX, MAX_SIZE};
	//forbiden = Location parameter
	std::string const forbiddenParam[] = {ALLOW_M, AUTOID, CGI_E, CGI_P, TRY, UPLOAD};

	for (int i = 0; i < 5; i++)
	{
		
		if (this->_tempServerConfigMap.find(mandatoryParam[i]) == this->_tempServerConfigMap.end())
			throw std::runtime_error(ERR_SERV_DIRECTIVE_MISSING(mandatoryParam[i]));
	}

	for (int i = 0; i < 6; i++)
	{
		if (this->_tempServerConfigMap.find(forbiddenParam[i]) != this->_tempServerConfigMap.end())
			throw std::runtime_error(ERR_SERV_FORBIDDEN_DIRECTIVE(forbiddenParam[i]));
	}

}



void	Parser::_checkDirectiveValue(){

	//DEBUG
	// std::cout << "in :: '_checkDirectiveValue'" << std::endl;

	//first check and get the 'root' directive because this path is needed for further checks
	// find 'root' path in the map, check it and put it in a '_tempRootDirPath'
	std::map<std::string, std::string>::iterator it = _tempServerConfigMap.find(ROOT_LOC);
	if (it == _tempServerConfigMap.end()) {
		throw std::runtime_error(ERR_SERV_DIRECTIVE_MISSING(std::string(ROOT_LOC)));
	}
	this->_checkRoot(it->second);
	this->_tempRootDirPath = it->second;



	//then check other directive
	for (std::map<std::string, std::string>::iterator it = _tempServerConfigMap.begin(); it != _tempServerConfigMap.end(); ++it) {
		if (it->second == SERVER) {
			//_checkServer should chekcthe closing parenthesis, but it's already done before
			// this->_checkServer();
			continue;
		}
		else if (it->first == LISTEN) {
			this->_checkListen(it->second);
		}
		else if (it->first == HOST) {
			this->_checkHost(it->second);
		}
		else if (it->first == ROOT_LOC) {
			//nothing, already done;
		}
		else if (it->first == INDEX) {
			this->_checkIndex(it->second);
		}
		else if (it->first == MAX_SIZE) {
			this->_checkMaxSize(it->second);
		}
		else if (it->first == SERVER_N) {
			this->_checkServerN(it->second);
		}
		else if (it->first == ERROR_P) {
			this->_checkErrorP(it->second);
		}
		//DEBUG
		// std::cout << "in :: '_checkDirectiveValue'" << std::endl;
		// std::cout << "it->second = " << it->second <<std::endl;
	}	
}

void Parser::_checkRoot(std::string& dirValue) {

	/*function seems identical to _checkReturn !!! probably possible to use it*/ 

	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	this->_checkPath(dirValue, true);
}


//---Directives checking functions---//

void Parser::_checkListen(std::string& dirValue) {
		// delete ending ';' if necessary to get a cleaner string later
		this->_delEndSemiColon(dirValue);

		// check if integer
		char* dirValueStr;
		long port = std::strtol(dirValue.c_str(), &dirValueStr, 10);

		// check if port is between 0 and 65535
		if (*dirValueStr != '\0' || port < 0 || port > 65535) {
			throw std::runtime_error(ERR_PORT_INPUT(dirValue));
		}
}

void Parser::_checkHost(std::string& dirValue) {
	this->_delEndSemiColon(dirValue);

	if (!this->_isValidHostName(dirValue) && !this->_isValidIpAddress(dirValue)) {
		throw std::runtime_error(ERR_HOST_INPUT(dirValue));
	}
}

bool Parser::_isValidHostName(const std::string& str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (!std::isalnum(*it) && *it != '.' && *it != '-') {
			return false;
		}
	}
	return !str.empty();
}


bool Parser::_isValidIpAddress(const std::string& str) {
	size_t pos = 0;
	int octet;
	int count = 0;

	while (count < 4) {
		size_t nextPos = str.find('.', pos);
		std::string segment = (nextPos == std::string::npos) ? str.substr(pos) : str.substr(pos, nextPos - pos);

		// Vérifie que chaque segment est composé uniquement de chiffres et est compris entre 0 et 255
		for (size_t i = 0; i < segment.size(); ++i) {
			if (!std::isdigit(segment[i])) return false;
		}

		octet = std::atoi(segment.c_str());
		if (octet < 0 || octet > 255) {
			return false;
		}

		if (nextPos == std::string::npos) break;
		pos = nextPos + 1;
		count++;
	}
	return (count == 3);
}



void Parser::_checkIndex(std::string& dirValue) {
	// delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	// make fullPath to the file
	std::string fullPath = this->_tempRootDirPath + "/" + dirValue;
	// std::cout << fullPath << std::endl;

	// check if the file exists
	int fd = open(fullPath.c_str(), O_RDONLY);
	if (fd < 0) {
		throw std::runtime_error(ERR_FILE(fullPath));
	}
	close(fd);
}


void Parser::_checkMaxSize(std::string& dirValue) {
		// Remove the ending semicolon
		this->_delEndSemiColon(dirValue);

		std::string units = "KMG"; // Accepted units
		size_t length = dirValue.length();
		size_t pos = 0;

		// Check if the number is valid
		std::string numberStr;
		while (pos < length && std::isdigit(dirValue[pos])) {
			numberStr += dirValue[pos++];
		}

		// Ignore spaces after the number
		while (pos < length && std::isspace(dirValue[pos])) {
			pos++;
		}

		// Check if the unit is valid (optional)
		char unit = '\0';
		if (pos < length) {
			unit = dirValue[pos++];
			if (units.find(unit) == std::string::npos) {
				throw std::runtime_error(ERR_MAX_SIZE_INPUT(dirValue));
			}
		}

		// Check that there are no characters after the unit
		while (pos < length && std::isspace(dirValue[pos])) {
			pos++;
		}
		if (pos < length) {
			throw std::runtime_error(ERR_MAX_SIZE_INPUT(dirValue));
		}

		// Check that the number is positive
		if (numberStr.empty() || ft_stoll(numberStr) <= 0) {
			throw std::runtime_error(ERR_MAX_SIZE_RANGE(dirValue));
		}

		// Convert the number to long 
		long sizeValue = ft_stoll(numberStr); 

		// Apply the factor based on the unit (if provided)
		if (unit == 'K') {
			sizeValue *= 1024; // Kilo
		} else if (unit == 'M') {
			sizeValue *= 1024 * 1024; // Mega
		} else if (unit == 'G') {
			sizeValue *= 1024 * 1024 * 1024; // Giga
		}

		//long long doesn't exist in CPP98
		// const long long max_size = 10LL * 1024 * 1024 * 1024; // 10G in bytes
		// if (sizeValue > max_size) {
		// 	throw std::runtime_error(ERR_MAX_SIZE_RANGE(dirValue));
		// }

		// Check that the size does not exceed 10G (= 5 * 2 GB)
		const long max_chunk_size = 2L * 1024L * 1024L * 1024L; // 2 GB in bytes
		const int num_chunks = 5;
		if (sizeValue > max_chunk_size * num_chunks) {
			throw std::runtime_error(ERR_MAX_SIZE_RANGE(dirValue));
		}


		// std::cout << "Valid size: " << dirValue << " (in bytes: " << sizeValue << ")" << std::endl;

		 // Convert the number to long 
		dirValue = numberStr;
	}

/**
 * @brief The function check the validity of the directive 'server_name'
 * It should contains only alphanumeric, hyphens, and periods characteres.
 * 
 * !!! But it could containes mutliple server_name, so the space character is authorized and considerd as a separator.
 * The list of server_name will be parsed during the server instantiation.
 * 
 * @param dirValue: the value of the 'server_name' directive which is a string. 
 */
void Parser::_checkServerN(std::string& dirValue) {
		// Remove the ending semicolon
		this->_delEndSemiColon(dirValue);

		// Check that the server name is valid (alphanumeric, hyphens, and periods)
		if (dirValue.empty()) {
			throw std::runtime_error(ERR_INVALID_SERVER_NAME(dirValue));
		}
		//DEBUG
		// std::cout << "in checkServerN:: ici" << std::endl;
		for (size_t i = 0; i < dirValue.length(); ++i) {
			char c = dirValue[i]; // Get the character at index i
			// Check for valid characters
			if (!std::isalnum(c) && c != '-' && c != '.' && c != ' ') {
				throw std::runtime_error(ERR_INVALID_SERVER_NAME(dirValue));
			}
		}

}

void Parser::_checkErrorP(std::string& dirValue) {
		// delete ending ';' if necessary to get a cleaner string later
		this->_delEndSemiColon(dirValue);

		//OLD FASHIONED for root
		// find 'root' path in the map
		// std::map<std::string, std::string>::const_iterator it = _tempServerConfigMap.find(ERROR_P);
		// if (it == _tempServerConfigMap.end()) {
		// 	throw std::runtime_error(ERR_SERV_DIRECTIVE_MISSING(dirValue));
		// }
		// // Get path
		// std::string rootDir = it->second; 

		// make fullPath to the file
		std::string fullPath = this->_tempRootDirPath + "/" + dirValue;
		// std::cout << fullPath << std::endl;

		// Vérifier si le fichier existe
		int fd = open(fullPath.c_str(), O_RDONLY);
		if (fd < 0) {
			throw std::runtime_error(ERR_FILE(fullPath));
		}

		//??? need to check if the file is '.html' and readble as html file ?

		// close fd
		close(fd);
}
