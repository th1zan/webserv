/**
 * @file Parser.cpp
 * @brief 
 */

#include "Parser.hpp"

Parser::Parser(int argc, char **argv){
	//check arguments
	this->_checkInputArg(argc, argv);

	//open confFile as ifstream object
	this->_confFile.open(this->_confFilePath.c_str());
	if (!this->_confFile.is_open())
		throw std::runtime_error(ERR_OPEN + this->_confFilePath);


	//init Parser instance variable
	// this->_line = 1;
	// this->_bracket = 0;
	// this->_hasServer = false;

	//parse config file
	this->_parseFile();

	
	//DEBUG
// 	size_t i = 0;
// 	for (std::vector<Server>::iterator it = this->_serversVector.begin(); it != this->_serversVector.end(); ++it){
// 		std::cout <<std::endl << "------ SERVERS PARAM AFTER PARSING -----" << std::endl;
// 		std::cout << "Serveur n°: " << i << std::endl;
// 		it->printServers();
// 		i++;
// 	}
}

Parser::~Parser(){

}

std::vector<Server>& Parser::getServersVector(){
	return(this->_serversVector);
}

void Parser::_parseFile() {
	std::string line;
	// size_t _nbLine = 0; //not used for the moment

	// Read confFile line by line
	while (std::getline(this->_confFile, line)) {
		std::istringstream iss(line);
		std::string word;
		std::vector<std::string> wordsVect;
		
		//DEBUG
		// std::cout << "parsed line: '" << line << "'" <<std::endl;

		// Read each word in the current line and push it in a vector
		while (iss >> word)
			wordsVect.push_back(word);

		//Analyze token's wordsVect to define token's category and push it to _tokensList
		Token tmpToken;


		if (wordsVect.empty()) {
			tmpToken.type = TK_EMPTY;
			tmpToken.value = "";
			this->_tokensVector.push_back(tmpToken);
		}
		else if (wordsVect[0][0] == '#') {
			tmpToken.type = TK_COMMENT;
			tmpToken.value = line;
			this->_tokensVector.push_back(tmpToken);
		}
		else if ((wordsVect.size() == 2 && wordsVect[0] == "server" && wordsVect[1] == "{") ||
				(wordsVect.size() == 1 && wordsVect[0] == "server{")) {
			tmpToken.type = TK_SERVER;
			tmpToken.value = line;
			this->_tokensVector.push_back(tmpToken);
		}
		else if ((wordsVect.size() == 2 && wordsVect[0] == "location" && wordsVect[1].back() == '{') ||
				(wordsVect.size() == 3 && wordsVect[0] == "location" && wordsVect[2] == "{")) {
			tmpToken.type = TK_LOCATION;
			tmpToken.value = line;
			this->_tokensVector.push_back(tmpToken);
		}
		else if (wordsVect[0] == "}") {
			tmpToken.type = TK_CLOSE_BRACKET;
			tmpToken.value = line;
			this->_tokensVector.push_back(tmpToken);
		}
		else if ((wordsVect.size() == 2)
				||(wordsVect.size() == 3)
				||(wordsVect.size() == 4)) {
			tmpToken.type = TK_TOKEN;
			tmpToken.value = line;
			if (line.back() != ';')
				throw std::runtime_error(ERR_SEMICOLON(line));
			this->_tokensVector.push_back(tmpToken);
		}

		else{
			//DEBUG
			// std::cout << "in: '_parseFile'" << std::endl;
			throw std::runtime_error(ERR_INVALID_KEY(line));
		}

		//DEBUG
		// std::cout << "in _parseFile:: tmpToken " << tmpToken.type << " - " << tmpToken.value <<std::endl;
				
	}
	this->_checkBracket();
	this->_getConfigAndInitServers();
	
}

void Parser::_checkBracket() {
	std::vector<eToken> stack;

	for (size_t i = 0; i < _tokensVector.size(); ++i) {
		const Token& token = this->_tokensVector[i];

		//add token to the stack if it is an "opening token"
		if (token.type == TK_SERVER || token.type == TK_LOCATION) {
			stack.push_back(token.type); 
		} else if (token.type == TK_CLOSE_BRACKET) {
			//check the stack is empty in cas of a "closing token"
			if (stack.empty())
				throw std::runtime_error(ERR_CLOSING_BRACKETS);
			stack.pop_back();
		}
	}

	// check if the stack is not empty (so unclosed bracket)
	if (!stack.empty()) {
		throw std::runtime_error(ERR_OPENING_BRACKET);
	}
}

void Parser::_getConfigAndInitServers(){
	//DEBUG
	// std::cout << "in :: '_getConfigAndInitServers': " << std::endl;
	
	this->_nServer = 0;
	this->_nbLine = 0;

	//for each line
	for (; this->_nbLine < this->_tokensVector.size(); this->_nbLine++) {

		//DEBUG
		// std::cout << "print _nbline: " << this->_nbLine 
		// << ", _tokensVector[this->_nbLine].type: " << _tokensVector[this->_nbLine].type
		// << ", _tokensVector[this->_nbLine].value: " << _tokensVector[this->_nbLine].value
		// << std::endl;
	
		//when a SERVER token encounterd we enter in a bloc server
		if (this->_tokensVector[this->_nbLine].type == TK_SERVER){
			
			//Get server configuration (= directives names / values) 
			this->_getConfigFromTokens();
			
			//Check configuration (Server and Location)
			this->_checkConfigs();

			// DEBUG
			// std::cout << std::endl << "in :: '_getConfigAndInitServers': " << std::endl;
			// std::cout << "Server n°: " << this->_nServer << std::endl;
			// printMap(_tempServerConfigMap);

			
			//DEBUG
			// for (std::vector<std::map<std::string, std::string> >::iterator it = _tempLocationMapVector.begin(); it != _tempLocationMapVector.end(); ++it) {
			// 	std::cout << std::endl;
			// 	std::cout << "new Location:" << std::endl;
			// 	printMap(*it);
			// }

			//Server instanciation with their parametres (include their location(s))

			
			this->_serversVector.push_back(Server(this->_serversVector, this->_tempServerConfigMap, this->_tempLocationMapVector));
			
			//DEBUG
			// std::cout << "in _getConfigAndInitServers:: " << this->_nServer<< " server(s) initialized" << std::endl;
			
			this->_tempLocationMapVector.clear();
			this->_tempServerConfigMap.clear();
			this->_tempRootDirPath.clear();
			this->_nServer++;
			
		}
		this->_nbLine = this->_nbLine - 1;
	}
	if (this->_nServer++ == 0)
		throw std::runtime_error(ERR_NO_SERVER_CONFIG);
	//DEBUG
	// std::cout << this->_nServer<< "server(s) initialized" << std::endl;
}

void Parser::_getConfigFromTokens(){
	//DEBUG
	// std::cout << "in :: '_getConfigFromTokens': " << std::endl;

	this->_nbLine++;
	//We are afer a SERVER token so in a server bloc. Now we seeks TOKEN or LOCATION
	for (;this->_nbLine< this->_tokensVector.size(); this->_nbLine++) {
		
		//DEBUG
		// std::cout << "[BEGIN] print _nbline: " << this->_nbLine 
		// << ", .type: " << _tokensVector[this->_nbLine].type
		// << ", .value: " << _tokensVector[this->_nbLine].value
		// << std::endl;
		
		if (this->_tokensVector[this->_nbLine].type == TK_TOKEN)
		{
			//DEBUG
			// std::cout << "[GO TO] TOKEN" << std::endl;

			this->_getServerParam();
		}
		else if (this->_tokensVector[this->_nbLine].type == TK_LOCATION)
		{
			//DEBUG
			// std::cout << "[GO TO] LOCATION" << std::endl;

			this->_getLocationParam();

			//add the _tempServerConfigMap.LocationMap to _tempLocationMapVector
			this->_tempLocationMapVector.push_back(this->_tempLocationConfigMap);
			
			
			//DEBUG
			// std::cout << "new _tempLocationConfigMap:" << std::endl;
			// printMap(_tempLocationConfigMap);


			this->_tempLocationConfigMap.clear();
		}
		else if (this->_tokensVector[this->_nbLine].type == TK_SERVER){
			//DEBUG
			// std::cout << "[RETURN] END SERVER BLOC" << std::endl;
			return;//end of server bloc
		}
		
		//DEBUG
		// std::cout << "[END] print _nbline: " << this->_nbLine 
		// << ", .type: " << _tokensVector[this->_nbLine].type
		// << ", .value: " << _tokensVector[this->_nbLine].value
		// << std::endl;
	}
	
	
}

void Parser::_getServerParam(){
	for (;this->_nbLine< this->_tokensVector.size(); this->_nbLine++) {
				
		if (this->_tokensVector[this->_nbLine].type == TK_CLOSE_BRACKET
			|| this->_tokensVector[this->_nbLine].type == TK_LOCATION)
		{
				this->_nbLine = this->_nbLine-1;
				return;//end of Server bloc
		}

		//put the parameters in a map . 
		this->_getParamFromToken(TK_SERVER);
	}
}

void Parser::_getLocationParam(){
	for (;this->_nbLine< this->_tokensVector.size(); ++this->_nbLine) {
			
		if (this->_tokensVector[this->_nbLine].type == TK_CLOSE_BRACKET)
			break;//end of Location bloc

		//put the parameters in a map
		this->_getParamFromToken(TK_LOCATION);
	}

}


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

void	Parser::_checkLocationParam(){

	this->_checkLocDirName();
	this->_checkLocDirValue();
	
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





void Parser::_checkRoot(std::string& dirValue) {

	/*function seems identical to _checkReturn !!! probably possible to use it*/ 

	// Delete ending ';' if necessary to get a cleaner string later
	this->_delEndSemiColon(dirValue);

	this->_checkPath(dirValue, true);
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
		if (numberStr.empty() || std::stoll(numberStr) <= 0) {
			throw std::runtime_error(ERR_MAX_SIZE_RANGE(dirValue));
		}

		// Convert the number to long 
		long sizeValue = std::stoll(numberStr); 

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
			if (!std::isalnum(c) && c != '-' && c != '.') {
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


/*
// Function probably not needed

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



void Parser::_delEndSemiColon(std::string& s){
	// delete last character (;)
	if (!s.empty() && s.back() == ';')
			s.pop_back();
}




void Parser::_getParamFromToken(int enumToken){
	
	//DEBUG
	// std::cout << "in :: '_getParamFromToken'::" << std::endl;

	if (this->_tokensVector[this->_nbLine].type == TK_EMPTY
	|| this->_tokensVector[this->_nbLine].type == TK_CLOSE_BRACKET
	|| this->_tokensVector[this->_nbLine].type == TK_COMMENT)
		return;

	std::string line = this->_tokensVector[this->_nbLine].value;
	 //DEBUG
	// std::cout << "print _nbline: " << this->_nbLine 
	// << ", enumToken: " << enumToken
	// << ", _tokensVector[this->_nbLine].type: " << _tokensVector[this->_nbLine].type
	// << ", _tokensVector[this->_nbLine].value: " << _tokensVector[this->_nbLine].value
	// << std::endl;



	std::istringstream iss(line);
	std::string dirName, dirValue;
	
	//!!! Hypothesis: There is at least a space between parameters. Ex: "location_space_.py_space_{"

	//get first and second word (of each line) as a directives' name and value
	iss >> dirName;
	// iss >> dirValue;

	//go through the spaces
	while (iss.peek() == ' ')
		iss.get();
	// get the rest of the line (the value)
	std::getline(iss, dirValue);

	//DEBUG
	// std::cout << "in: '_getParamFromToken': dirName= '" << dirName << "', dirValue= '" << dirValue << "'" <<std::endl;

	//check each directives'name
	if (dirName != ALLOW_M && dirName != AUTOID && dirName != CGI_E && dirName != CGI_P &&
	dirName != MAX_SIZE && dirName != ERROR_P && dirName != HOST && dirName != INDEX &&
	dirName != LISTEN && dirName != LOCATION && dirName != RETURN && dirName != ROOT_LOC &&
	dirName != SERVER_N && dirName != TRY && dirName != UPLOAD && dirName != SERVER)
	{
		throw std::runtime_error(ERR_INVALID_KEY(line));
	}


	//No more checks here.	

	//DEBUG
	// std::cout << "in: '_getParamFromToken':: enumToken: " << enumToken << ", dirName= '"<< dirName << "', dirValue= '" << dirValue << "'" <<std::endl;

	//map each token in a _tempServerConfigMap or a _tempLocationConfigMap
	if (enumToken == TK_SERVER)
		this->_tempServerConfigMap[dirName] = dirValue;
	else if (enumToken == TK_LOCATION){
		this->_tempLocationConfigMap[dirName] = dirValue;
	}
	dirName.clear();
	dirValue.clear();
}


void Parser::_checkInputArg(int argc, char **argv){
	//1.check argument
	if (argc > 2)
		throw std::runtime_error(ERR_ARG);
	else if (argc == 2)
		this->_confFilePath = argv[1];
	else
		this->_confFilePath = DEFAULT_CONF;

	//2.get configuration file name
	std::string confFilename;
	size_t lastSlash;

	lastSlash = this->_confFilePath.find_last_of("/");
	if (lastSlash == std::string::npos) //aucun nom
		confFilename = this->_confFilePath;
	else
		confFilename = this->_confFilePath.substr(lastSlash + 1);
	
	//3.check extension
	std::string extension;
	size_t lastDot;
	lastDot = confFilename.find_last_of(".");
	if (lastDot != std::string::npos)
		extension = confFilename.substr(lastDot + 1);
	else
		extension = "";
	if(extension != "conf")
		throw std::runtime_error(ERR_FILE_CONF(confFilename));
	
}
