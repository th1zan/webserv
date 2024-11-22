/**
 * @file Parser_2_checkServer.cpp
 * @brief This file containes the functions used to check the 'Server' parameters except for its Location parameters.
 */

#include "Parser.hpp"

/**
 * The function _checkConfigs() calls _checkServerParam() and _checkLocationParam() to validate
 * server and location parameters.
 */
void Parser::_checkConfigs(){
	this->_checkServerParam();
	this->_checkLocationParam();
}


/**
 * The function `_checkServerParam` checks the directive name and value.
 */
void	Parser::_checkServerParam(){
	this->_checkDirectiveName();
	this->_checkDirectiveValue();
}


/**
 * The function `_checkDirectiveName` ensures that certain mandatory parameters are present and that
 * certain forbidden parameters are not present in a server configuration map.
 */
void	Parser::_checkDirectiveName(){
	std::string const mandatoryParam[] = {LISTEN, HOST, ROOT_LOC, INDEX, MAX_SIZE};
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


/**
 * The function `_checkDirectiveValue` iterates through a map of server configuration directives,
 * checking each one for validity and performing specific checks based on the directive type.
 */

void	Parser::_checkDirectiveValue(){

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
			//_checkServer should chekc the closing parenthesis, but it's already done before
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
			//nothing, check already done;
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
		else if (it->first.find(ERROR_P) == 0) {
			this->_checkErrorP(it->second);
		}
	}	
}

/**
 * The function `_checkRoot` removes the trailing semicolon from a string and checks if it is a valid
 * path.
 * 
 * @param dirValue The `dirValue` parameter in the `_checkRoot` function is a reference to a
 * `std::string` variable. It is used within the function to perform certain operations like removing a
 * trailing semicolon and checking the path.
 */
void Parser::_checkRoot(std::string& dirValue) {

	/*function seems identical to _checkReturn !!! probably possible to use it*/ 
	this->_delEndSemiColon(dirValue);
	this->_checkPath(dirValue, true);
}
/**
 * The function `_checkListen` validates if a given string represents a valid port number within
 * the range of 0 to 65535.
 * 
 */

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

/**
 * The function `_checkHost` validates whether a given string is a valid host name or IP address and
 * throws a runtime error if it is not.
 * 
 */
void Parser::_checkHost(std::string& dirValue) {
	this->_delEndSemiColon(dirValue);

	if (!this->_isValidHostName(dirValue) && !this->_isValidIpAddress(dirValue)) {
		throw std::runtime_error(ERR_HOST_INPUT(dirValue));
	}
}

/**
 * The function checks if a given string is a valid hostname by ensuring that it only contains
 * alphanumeric characters, dots, and hyphens.
 * 
 * @return The function `_isValidHostName` returns a boolean value, indicating whether the input string
 * `str` is a valid host name or not.
 */
bool Parser::_isValidHostName(const std::string& str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (!std::isalnum(*it) && *it != '.' && *it != '-') {
			return false;
		}
	}
	return !str.empty();
}


/**
 * The function `_isValidIpAddress` checks if a given string represents a valid IPv4 address with four
 * octets separated by periods.
 * 
 * @return a boolean value indicating whether the input string is a valid IP address. If the input
 * string contains exactly 4 segments separated by '.', and each segment is composed only of digits and
 * falls within the range of 0 to 255, the function returns true. Otherwise, it returns false.
 */
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

/**
 * The function `_checkIndex` checks if a file exists by constructing a full path and attempting to
 * open it.
 * 
 * @param dirValue `dirValue` is a string that represents a directory value. It is used to construct
 * the full path to a file by appending it to the temporary root directory path. The function
 * `_checkIndex` then checks if the file specified by the constructed full path exists by attempting to
 * open it for reading.
 */
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
/**
 * The function `_checkMaxSize` in C++ parses a string representing a size value with optional units
 * (K, M, G) and checks if it is within a specified range.
 * 
 * @param dirValue The code snippet you provided is a function named `_checkMaxSize` in a class called
 * `Parser`. This function is responsible for parsing a string representing a size value with optional
 * units (K, M, G) and performing various checks on the input.
 */
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
		// Check that the size does not exceed 10G (= 5 * 2 GB)
		const long max_chunk_size = 2L * 1024L * 1024L * 1024L; // 2 GB in bytes
		const int num_chunks = 5;
		if (sizeValue > max_chunk_size * num_chunks) {
			throw std::runtime_error(ERR_MAX_SIZE_RANGE(dirValue));
		}

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
	
		for (size_t i = 0; i < dirValue.length(); ++i) {
			char c = dirValue[i]; // Get the character at index i
			// Check for valid characters
			if (!std::isalnum(c) && c != '-' && c != '.' && c != ' ') {
				throw std::runtime_error(ERR_INVALID_SERVER_NAME(dirValue));
			}
		}

}

/**
 * The function `_checkErrorP` parses a string to extract values, validates them, and checks if the
 * last value corresponds to a valid HTML file path.
 * 
 */

void Parser::_checkErrorP(std::string& dirValue) {
	// delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	std::istringstream iss(dirValue);
	std::string tmpErrorP;
	std::string lastValue;

	// Extract all values into a vector
	std::vector<std::string> values;
	while (iss >> tmpErrorP) {
		//DEBUG
		// std::cout << "in : _checkErrorP():: tmpErrorP: " << tmpErrorP << std::endl;
		values.push_back(tmpErrorP);
	}
	
	//DEBUG : normally this condition is not necessary, to delete before production
	if (values.empty()) {
	throw std::runtime_error("Error: No values provided");
	}

	// Check all values except the last one
	for (size_t i = 0; i < values.size() - 1; ++i) {
		if (!this->_isNumber(values[i])) {
			throw std::runtime_error("Error: Expected a number but got " + values[i]);
		}
	}

	// The last value should be a valid HTML file path
	lastValue = values.back();
	std::string fullPath = this->_tempRootDirPath + "/" + lastValue;

	// Check if the file exists and is readable
	int fd = open(fullPath.c_str(), O_RDONLY);
	if (fd < 0) {
		throw std::runtime_error(ERR_FILE(fullPath));
	}

	// Check if the file is an HTML file based on extension
	if (fullPath.substr(fullPath.find_last_of(".") + 1) != "html") {
		close(fd);
		throw std::runtime_error("Error: The file is not an HTML file: " + fullPath);
	}

	// Close the file descriptor
	close(fd);
}
