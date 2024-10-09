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

	// Read confFile line by line
	while (std::getline(this->_confFile, line)) {
		std::istringstream iss(line);
		std::string word;
		std::vector<std::string> words;
		
		if (line.empty())
			continue;

		// Read each word in the current line
		while (iss >> word)
			words.push_back(word);

		//Analyze token's words to define token's category and push it to _tokensList
		if (words.size() == 0) //
			continue;
		else if (words[0][0] == '#')
			this->_tokensVector.push_back({COMMENT, line});
		else if (words[0] == "server") {
			if (words.size() == 2 && words[1] == "{")
				this->_tokensVector.push_back({SERVER, line});
			else
				this->_tokensVector.push_back({ERROR, line});
		}
		else if (words[0] == "location") {
			if (words.size() == 3 && words[2] == "{")
			this->	_tokensVector.push_back({LOCATION, line});
			else
				this->_tokensVector.push_back({ERROR, line});
		}
		else if (words[0] == "}")
			this->_tokensVector.push_back({CLOSE_BRACKET, line});
		else if (words.size() == 2 && line.back() == ';')
			this->_tokensVector.push_back({TOKEN, line});
		else if (words.size() == 3 && line.back() == ';')
			this->_tokensVector.push_back({TOKEN, line});
		else
			this->_tokensVector.push_back({ERROR, line});
	
	}

	this->_checkTokens();
	this->_getServerConfigFromTokens();
	this->_checkConfigs();

}

void Parser::_getServerConfigFromTokens(){
	size_t n = 0;
	for (size_t i = 0; i < this->__tokensVector.size(); ++i) {
		const Token& token = this->_tokensVector[i];
		if (token.type == TOKEN){}
			this->_serversVector[n];
		if (token.type == LOCATION)
			_getLocationConfig;
	}

}

void Parser::_checkTokens(){
	int serverCount = 0;
	int locationCount = 0;
	int closeBracketCount = 0;

	// iteration on each token to count the open and closing bracket
	for (size_t i = 0; i < _tokensList.size(); ++i) {
		const Token& token = _tokensList[i];
		if (token.type == ERROR)
			throw std::runtime_error(ERR_NO_SERVER_CONFIG);
		if (token.type == SERVER)
			++serverCount;
		if (token.type == LOCATION)
			++locationCount;
		if (token.type == CLOSE_BRACKET)
			++closeBracketCount;
	}

	// Check opening and closing parenthesis
	if (serverCount + locationCount != closeBracketCount)
		throw std::runtime_error(ERR_UNCLOSED_BRACKETS);

	// Check if there is at least one server
	if (serverCount == 0)
		throw std::runtime_error(ERR_NO_SERVER_CONFIG);
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