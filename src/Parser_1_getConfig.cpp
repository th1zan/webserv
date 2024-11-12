/**
 * @file Parser_1_getConfig.cpp
 * @brief This file containes the functions used to get the configuration from each token
 */

#include "Parser.hpp"


/**
 * @brief This function loop on the tokens and if the token's type is :
 * -  TK_SERVER then it gets the server's parameters from the next token and put them in a temporay Server instance.
 * -  TK_LOCATION then it gets the locations's parameters from the next token and put them in a temporay Location map.
 * 
 * At the end instantiate a server with the temp Server and temp Location map.
 * 
 */
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

void Parser::_delEndSemiColon(std::string& s){
	// delete last character (;)
	if (!s.empty() && s[s.size() - 1] == ';')
			s.erase(s.size() - 1);
}


/**
 * @brief Extracts and processes a parameter (= directive) from the current token, store it in a temporary map
 *  based on the token type (server or location).
 *
 * This function parses the current line in `_tokensVector` identified by `_nbLine`, extracting a directive name (`dirName`)
 * and its corresponding value (`dirValue`). It verifies the directive name is valid and stores it in either
 * `_tempServerConfigMap` or `_tempLocationConfigMap`, depending on the `enumToken` type.
 *
 * @param enumToken Indicates the type of configuration expected (e.g., TK_SERVER or TK_LOCATION),
 *                  determining the target map for storage.
 *
 * Main steps:
 * 1. Checks if the line is empty, a comment, or a close bracket; exits if true.
 * 2. Retrieves the line text (`line`) from `_tokensVector`.
 * 3. Uses `istringstream` to split the first word as `dirName` and the remainder as `dirValue`.
 * 4. Validates `dirName` against a predefined list; throws an exception if invalid.
 * 5. Based on `enumToken`, stores `dirName` and `dirValue` in `_tempServerConfigMap` (for TK_SERVER) or
 *    `_tempLocationConfigMap` (for TK_LOCATION).
 * 6. Clears `dirName` and `dirValue` to prepare for the next invocation.
 */

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
