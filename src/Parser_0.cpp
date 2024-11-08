/**
 * @file Parser_0.cpp
 * @brief This file containes _parseFile() function who tokenize each line of the configuration file.
 * Each token is a struct composed by:
 * - the line (a string)
 * - the type (an enum) : TK_EMPTY, TK_COMMENT, TK_SERVER, TK_LOCATION, TK_CLOSE_BRACKET, TK_TOKEN
 */

#include "Parser.hpp"

/**
 * @brief Instatiate a Parser object.
 * - check the input file
 * - init the parsing of the file
 * @param argc max 1 configuration file
 * @param argv the name of the configuration file
 */
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

// Getter
std::vector<Server>& Parser::getServersVector(){
	return(this->_serversVector);
}

/**
 * @brief The function parse each line from the config file and put them as a 'struct token' in a 'vectortokensVector'.
 * Each token struct is composed by:
 * - the line (a string)
 * - the type (an enum) : TK_EMPTY, TK_COMMENT, TK_SERVER, TK_LOCATION, TK_CLOSE_BRACKET, TK_TOKEN
 */
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

/**
 * @brief Check if there is a closing bracket for each blocks in the configuration file.
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
 * @brief Extract the settings from each token (=line of the configuration file) to set a configuration per Server bloc.
 * Then check configuration.
 * Then use the configuration to instantiate the 'Servers'
 * 
 */
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
