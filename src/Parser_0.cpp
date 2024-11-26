/**
 * @file Parser_0.cpp
 * @brief This file containes the _parseFile() function.
 * 
 * This function tokenizes each line of the configuration file to get the server configuration.
 * Each token is a struct composed by:
 * - the line (a string)
 * - the type (an enum) : TK_EMPTY, TK_COMMENT, TK_SERVER, TK_LOCATION, TK_CLOSE_BRACKET, TK_TOKEN
 */

#include "Parser.hpp"

/**
 * @brief Construct a new Parser:: Parser object
 * 
 * * This object will:
 * - check the input file
 * - init the parsing of the file
 * @param [in] argc max 1 configuration file
 * @param [in] argv the name of the configuration file
 */
Parser::Parser(int argc, char **argv){
	//check arguments
	this->_checkInputArg(argc, argv);

	//open confFile as ifstream object
	this->_confFile.open(this->_confFilePath.c_str());
	if (!this->_confFile.is_open())
		throw std::runtime_error(ERR_OPEN + this->_confFilePath);
		
	//parse config file
	this->_parseFile();
}

/**
 * @brief Destroy the Parser:: Parser object
 * 
 */
Parser::~Parser(){
}

/**
 * @brief Getter for the ServersVector
 * 
 * ServersVector contains all the Servers instances.
 * @return std::vector<Server>& 
 */
std::vector<Server>& Parser::getServersVector(){
	return(this->_serversVector);
}

/**
 * @brief The function parse the server configuration file.
 * 
 * Each line from the config file is put as a 'struct token' in a 'vectorTokensVector'.
 * 
 * Each token is a struct composed by:
 * - the line (a string)
 * - the type (an enum) : TK_EMPTY, TK_COMMENT, TK_SERVER, TK_LOCATION, TK_CLOSE_BRACKET, TK_TOKEN
 * 
 */
void Parser::_parseFile() {
	std::string line;

	// Read confFile line by line
	while (std::getline(this->_confFile, line)) {
		std::istringstream iss(line);
		std::string word;
		std::vector<std::string> wordsVect;

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
		else if ((wordsVect.size() == 2 && wordsVect[0] == "server" && wordsVect[1] == "{")
				|| (wordsVect.size() == 1 && wordsVect[0] == "server{")) {
			tmpToken.type = TK_SERVER;
			tmpToken.value = line;
			this->_tokensVector.push_back(tmpToken);
		}
		else if ((wordsVect.size() == 2 && wordsVect[0] == "location" && wordsVect[1][wordsVect[1].size() - 1] == '{')
				|| (wordsVect.size() == 3 && wordsVect[0] == "location" && wordsVect[2] == "{")) {
			tmpToken.type = TK_LOCATION;
			tmpToken.value = line;
			this->_tokensVector.push_back(tmpToken);
		}
		else if (wordsVect[0] == "}") {
			tmpToken.type = TK_CLOSE_BRACKET;
			tmpToken.value = line;
			this->_tokensVector.push_back(tmpToken);
		}
		// else if ((wordsVect.size() == 2)
		// 		||(wordsVect.size() == 3)
		// 		||(wordsVect.size() == 4))
		else { //the error_page directive needs to accept unlimited values on a line.
			tmpToken.type = TK_TOKEN;
			tmpToken.value = line;
			if (line[line.size() - 1] != ';')
				throw std::runtime_error(ERR_SEMICOLON(line));
			this->_tokensVector.push_back(tmpToken);
		}
		// else{
		// 	std::cout << "COUCOU" << std::endl;
		// 	throw std::runtime_error(ERR_INVALID_KEY(line));
		// 	std::cout << "SALUT" << std::endl;
		// }
	}
	this->_checkBracket();
	this->_getConfigAndInitServers();	
}

/**
 * @brief Check the closing brackets.
 * 
 * Check if there is a closing bracket for each blocks in the configuration file.
 * 
 */
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

/**
 * @brief Extract the settings from each token
 * 
 * Each token is a struct containing a string (the line of the configuration file) and an enum (its type).
 * Each `TK_SERVER` token means the begin of a server block. 
 * Therefore the following token are parsed to get this specific Server configuration.
 * Then check this configuration.
 * Then use the configuration to instantiate a 'Servers'
 * 
 */
void Parser::_getConfigAndInitServers(){	
	this->_nServer = 0;
	this->_nbLine = 0;

	//for each line
	for (; this->_nbLine < this->_tokensVector.size(); this->_nbLine++) {

		//when a SERVER token encounterd we enter in a bloc server
		if (this->_tokensVector[this->_nbLine].type == TK_SERVER){
			
			//Get server configuration (= directives names / values) 
			this->_getConfigFromTokens();
			
			//Check configuration (Server and Location)
			this->_checkConfigs();

			//Server instanciation with their parametres (include their location(s))
			this->_serversVector.push_back(Server(this->_serversVector, this->_tempServerConfigMap, this->_tempLocationMapVector));
			
			//delete the temporary datas and get the next TK_SERVER
			this->_tempLocationMapVector.clear();
			this->_tempServerConfigMap.clear();
			this->_tempRootDirPath.clear();
			this->_nServer++;
		}
		this->_nbLine = this->_nbLine - 1;
	}
	if (this->_nServer++ == 0)
		throw std::runtime_error(ERR_NO_SERVER_CONFIG);
}
