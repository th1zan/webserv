#include "Parser.hpp"

Parser::Parser(int argc, char **argv){
	//check arguments
	this->_checkInputArg(argc, argv);

	//open confFile as ifstream object
	this->_confFile.open(this->_confFilePath.c_str());
	if (!this->_confFile.is_open());
		throw std::runtime_error(ERR_OPEN + this->_confFilePath);

	//init Parser instance variable
	// this->_line = 1;
	// this->_bracket = 0;
	// this->_hasServer = false;

	//parse config file
	this->_parseFile();


}

Parser::~Parser(){

}

void Parser::_parseFile() {
	std::string line;
	size_t _nbLine = 0;

	// Read confFile line by line
	while (std::getline(this->_confFile, line)) {
		std::istringstream iss(line);
		std::string word;
		std::vector<std::string> wordsVect;
		
		// Read each word in the current line and push it in a vector
		while (iss >> word)
			wordsVect.push_back(word);

		//Analyze token's wordsVect to define token's category and push it to _tokensList
		if (wordsVect.empty())
				_tokensVector.push_back({TK_EMPTY, ""});
		else if (wordsVect[0][0] == '#')
				this->_tokensVector.push_back({TK_COMMENT, line});
		else if ((wordsVect.size() == 2 && wordsVect[0] == "server" && wordsVect[1] == "{")
			|| (wordsVect.size() == 1 && wordsVect[0] == "server{"))
				this->_tokensVector.push_back({TK_SERVER, line});
		else if ((wordsVect.size() == 2 && wordsVect[0] == "location" && wordsVect[1].back() == "{")
			|| (wordsVect.size() == 3 && wordsVect[0] == "server" && wordsVect[3] == "{"))
			// ||	(wordsVect.size() == 1 && wordsVect[0].substr(0, 8) == "location" && wordsVect[1].back() == "{"
				this->_tokensVector.push_back({TK_LOCATION, line});
		else if (wordsVect[0] == "}")
				this->_tokensVector.push_back({TK_CLOSE_BRACKET, line});
		else if ((wordsVect.size() == 2 && line.back() == ';')
			|| (wordsVect.size() == 3 && line.back() == ';'))
				this->_tokensVector.push_back({TK_TOKEN, line});
		else
				throw std::runtime_error(ERR_INVALID_KEY(this->_nbLine, line));
	}
	this->_checkBracket()
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
	this->_nServer = 0;
	this->_nbLine = 0;

	//for each line
	for (; this->_nbLine < this->_tokensVector.size(); this->_nbLine++) {
		
		//when a SERVER token encounterd we enter in a bloc server
		if (this->_tokensVector[this->_nbLine].type == TK_SERVER){
			
			//Get server configuration (= directives names / values) 
			this->_getConfigFromTokens();

			//Check configuaration
			this->_checkServerConfigs() 

			//Server instanciation with their parametres (include their location(s))
			this->_serversVector.push_back(Server(this->_serversVector,  this->_tempServerConfigMap, this->_tempLocationMap));

			this->_nServer++;
		}
	}
	if (this->_nServer++ == 0)
		throw std::runtime_error(ERR_NO_SERVER_CONFIG);
}

void Parser::_getConfigFromTokens(){
	this->_nbLine++;
	//in a bloc server, we seeks TOKEN or LOCATION
	for (;this->_nbLine< this->_tokensVector.size(); this->_nbLine++) {
		if (this->_tokensVector[this->_nbLine].type == TK_TOKEN)
			this->_getServerParam()
			this->_getParamFromToken(TK_SERVER);
		else if (this->_tokensVector[this->_nbLine].type == TK_LOCATION){
			this->_getLocationParam();

			//add the struct Location to the _tempServerConfigMap.LocationMap 
			this->_serversVector


		}
		else if (this->_tokensVector[this->_nbLine].type == TK_SERVER)
			break;//end of server bloc
		this->_nbLine++;


	}
	
	
}

void Parser::_getLocationParam(){
	for (;this->_nbLine< this->_tokensVector.size(); ++this->_nbLine) {
				//put the parameters in a map
				this->_getParamFromToken(TK_LOCATION);

				//check Location param
				this->_checkLocationParam();

				if (this->_tokensVector[this->_nbLine].type == TK_CLOSE_BRACKET)
					break;//end of Location bloc
			}
	

}

void	Parser::_checkLocationParam(){

// if (dirValue.back() != ';')
	// 	throw std::runtime_error(ERR_SEMICOLON(this->_nbLine, line));

	// // delete last character (;)
	// if (!dirValue.empty() && dirValue.back() == ';')
	// 		dirValue.pop_back();

}


void Parser::_getServerParam(){
	for (;this->_nbLine< this->_tokensVector.size(); ++this->_nbLine) {
				//put the parameters in a map
				this->_getParamFromToken(TK_SERVER);

				//check Server param
				this->_checkServerParam();

				if (this->_tokensVector[this->_nbLine].type == TK_CLOSE_BRACKET
					|| this->_tokensVector[this->_nbLine].type == TK_LOCATION)
					break;//end of Server bloc
			}
	

}

void	Parser::_checkServerParam(){

	// if (dirValue.back() != ';')
	// 	throw std::runtime_error(ERR_SEMICOLON(this->_nbLine, line));

	// // delete last character (;)
	// if (!dirValue.empty() && dirValue.back() == ';')
	// 		dirValue.pop_back();

}

void Parser::_getParamFromToken(int enumToken){
	std::string line = this->_tokensVector[this->_nbLine];
	std::istringstream iss(line);
	std::string dirName, dirValue;
	
	//!!! There is at least a space between parameters, included "location .py {"

	//get first and second word (of each line) as a directives' name and value
	iss >> dirName;
	iss >> dirValue;


	//check each directives'name
	if (dirName != ALLOW_M || dirName != AUTOID || dirName != CGI_E || dirName == CGI_P
		|| dirName != MAX_SIZE || dirName != ERROR_P || dirName != HOST || dirName != INDEX
		|| dirName != LISTEN || dirName != LOCATION || dirName != RETURN || dirName != ROOT
		|| dirName != SERVER_N || dirName != TRY || dirName != UPLOAD || dirName != SERVER)
		{
			throw std::runtime_error(ERR_INVALID_KEY(this->_nbLine, line));
		}


	

	//map each token in a _tempServerConfigMap or a _tempLocationConfigMap
	if (enumToken == TK_SERVER)
		this->_tempServerConfigMap[dirName] = value;
	else if (enumToken == TK_LOCATION)
		this->_tempLocationConfigMap[dirName] = value;
}




void Parser::_checkInputArg(int argc, char **argv){
	//1.check argument
	if (argc > 2)
		throw std::runtime_error(ERR_ARG);
	else if (argv == 2)
		this->_confFilePath = av[1];
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
		throw std::runtime_error("'" + confFilename + "'" + ERR_FILE);
	
}