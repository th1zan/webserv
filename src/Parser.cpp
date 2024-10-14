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
		{
				_tokensVector.push_back({TK_EMPTY, ""});
		}
		else if (wordsVect[0][0] == '#')
		{
				this->_tokensVector.push_back({TK_COMMENT, line});
		}
		else if ((wordsVect.size() == 2 && wordsVect[0] == "server" && wordsVect[1] == "{")
			|| (wordsVect.size() == 1 && wordsVect[0] == "server{"))
		{
				this->_tokensVector.push_back({TK_SERVER, line});
		}
		else if ((wordsVect.size() == 2 && wordsVect[0] == "location" && wordsVect[1].back() == "{")
			|| (wordsVect.size() == 3 && wordsVect[0] == "server" && wordsVect[3] == "{"))
		{
			// ||	(wordsVect.size() == 1 && wordsVect[0].substr(0, 8) == "location" && wordsVect[1].back() == "{"
				this->_tokensVector.push_back({TK_LOCATION, line});
		}
		else if (wordsVect[0] == "}"){
				this->_tokensVector.push_back({TK_CLOSE_BRACKET, line});
		}
		else if ((wordsVect.size() == 2 && line.back() == ';')
			|| (wordsVect.size() == 3 && line.back() == ';'))
		{
				this->_tokensVector.push_back({TK_TOKEN, line});
		}
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

			//Check configuration (Server and Location)
			this->_checkConfigs() 

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
	//We are afer a SERVER token so in a server bloc. Now we seeks TOKEN or LOCATION
	for (;this->_nbLine< this->_tokensVector.size(); this->_nbLine++) {
		if (this->_tokensVector[this->_nbLine].type == TK_TOKEN)
		{
			this->_getServerParam();
		}
		else if (this->_tokensVector[this->_nbLine].type == TK_LOCATION)
		{
			this->_getLocationParam();

			//add the _tempServerConfigMap.LocationMap to _tempLocationMapVector
			this->_tempLocationMapVector.push_back(this->_tempLocationConfigMap);
		}
		else if (this->_tokensVector[this->_nbLine].type == TK_SERVER)
			break;//end of server bloc
		this->_nbLine++;
	}
	
	
}

void Parser::_getServerParam(){
	for (;this->_nbLine< this->_tokensVector.size(); ++this->_nbLine) {
				//put the parameters in a map . 
				this->_getParamFromToken(TK_SERVER);
				if (this->_tokensVector[this->_nbLine].type == TK_CLOSE_BRACKET
					|| this->_tokensVector[this->_nbLine].type == TK_LOCATION)
					break;//end of Server bloc
			}
}

void Parser::_getLocationParam(){
	for (;this->_nbLine< this->_tokensVector.size(); ++this->_nbLine) {
				//put the parameters in a map
				this->_getParamFromToken(TK_LOCATION);

				if (this->_tokensVector[this->_nbLine].type == TK_CLOSE_BRACKET)
					break;//end of Location bloc
			}

}


void Parser::_checkConfigs(){
	this->_checkServerParam();
	this->_checkLocationParam();
}


void	Parser::_checkServerParam(){

	this->_checkDirectiveName();
	this->_checkDirectiveValue();
	

}

void	Parser::_checkDirectiveName(){

	std::string const mandatoryParam[] = {LISTEN, HOST, ROOT, INDEX, MAX_SIZE};
	//forbiden = Location parameter
	std::string const forbiddenParam[] = {ALLOW_M, AUTOID, CGI_E, CGI_P, TRY, UPLOAD};

	for (int i = 0; i < 5; i++)
	{
		if (_tempServerConfigMap.find(mandatoryParam[i]) == _tempServerConfigMap.end())
			throw std::runtime_error(ERR_DIRECTIVE_MISSING(mandatoryParam[i]));
	}

	for (int i = 0; i < 6; i++)
	{
		if (_tempServerConfigMap.find(forbiddenParam[i]) != _tempServerConfigMap.end())
			throw std::runtime_error(ERR_FORBIDDEN_DIRECTIVE(forbiddenParam[i]));
	}

}

void	Parser::_checkDirectiveValue(){

	for (std::map<std::string, std::string>::iterator it = _tempServerConfigMap.begin(); it != _tempServerConfigMap.end(); ++it) {
		if (it->second == SERVER) {
			//_checkServer should chekcthe closing parenthesis, but it's already done before
			// this->_checkServer();
			continue;
		}
		else if (it->second == LISTEN) {
			this->_checkListen(it->second);
		}
		else if (it->second == HOST) {
			this->_checkHost(it->second);
		}
		else if (it->second == ROOT) {
			this->_checkRoot(it->second);
		}
		else if (it->second == INDEX) {
			this->_checkIndex(it->second);
		}
		else if (it->second == MAX_SIZE) {
			this->_checkMaxSize(it->second);
		}
		else if (it->second == SERVER_N) {
			this->_checkServerN(it->second);
		}
		else if (it->second == ERROR_P) {
			this->_checkErrorP(it->second);
		}
	}
}

void	Parser::_checkLocationParam(){

	this->_checkLocDirName();
	this->_checkLocDirValue();
	
}

//---Directives checking functions---//

void Parser::_checkListen(std::string& dirValue) {
		// delete ending ';' if necessary to get a cleaner string later
		this->_evalDelEndSemiColon(dirValue);

		// check if integer
		char* dirValueStr;
		long port = std::strtol(dirValue.c_str(), &dirValueStr, 10);

		// check if port is between 0 and 65535
		if (*dirValueStr != '\0' || port < 0 || port > 65535) {
			throw std::runtime_error(ERR_PORT_INPUT(dirValue));
		}
}

void Parser::_checkHost(std::string& dirValue) {
		// delete ending ';' if necessary to get a cleaner string later
		this->_evalDelEndSemiColon(dirValue);


		// check if hostname or IP adress is valid using a regex
		std::regex host_regex(R"(^[a-zA-Z0-9.-]+$)");
		std::regex ip_regex(R"(^(\d{1,3}\.){3}\d{1,3}$)");

		if (!std::regex_match(dirValue, host_regex) && !std::regex_match(dirValue, ip_regex))
		{
			throw std::runtime_error(ERR_HOST_INPUT(dirValue));
		}

		//not really necessary ?!?

		// Check if each number in the ip adresse is between 0 and 255 (for ipV4)
		if (std::regex_match(dirValue, ip_regex)) {
			size_t pos = 0;
			int octet;
			int count = 0;

			while ((pos = dirValue.find('.', pos)) != std::string::npos) {
				octet = std::atoi(dirValue.substr(count, pos - count).c_str());
				if (octet < 0 || octet > 255) {
					throw std::runtime_error(ERR_HOST_INPUT(dirValue));
				}
				count = ++pos;
			}

			//check last octet
			octet = std::atoi(dirValue.substr(count).c_str());
			if (octet < 0 || octet > 255) {
				throw std::runtime_error(ERR_HOST_INPUT(dirValue));
			}
		}
}

void Parser::_checkRoot(std::string& dirValue) {
		// delete ending ';' if necessary to get a cleaner string later
		this->_evalDelEndSemiColon(dirValue);

		// Try to open the path
		int fd = open(dirValue.c_str(), O_RDONLY);
		if (fd < 0) {
			throw std::runtime_error(ERR_DIRECTORY(dirValue));
		}

		// Si l'ouverture a réussi, nous devons fermer le descripteur de fichier
		close(fd);
}

void	Parser::_checkIndex(std::string& dirValue) {
		// delete ending ';' if necessary to get a cleaner string later
		this->_evalDelEndSemiColon(dirValue);

		// find 'root' path in the map
		auto it = _tempServerConfigMap.find(ROOT);
		if (it == _tempServerConfigMap.end()) {
			throw std::runtime_error(ERR_DIRECTIVE_MISSING(ROOT));
		}
		
		// Get path
		std::string rootDir = it->second; 

		// make fullPath to the file
		std::string fullPath = rootDir + "/" + dirValue;

		// Vérifier si le fichier existe
		int fd = open(fullPath.c_str(), O_RDONLY);
		if (fd < 0) {
			throw std::runtime_error(ERR_FILE(fullPath));
		}

		// Fermer le descripteur de fichier si l'ouverture a réussi
		close(fd);
}
void Parser::_checkMaxSize(std::string& dirValue) {
		// Remove the ending semicolon
		this->_evalDelEndSemiColon(dirValue);

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

		// Convert the number to long long
		long long sizeValue = std::stoll(numberStr);

		// Apply the factor based on the unit (if provided)
		if (unit == 'K') {
			sizeValue *= 1024; // Kilo
		} else if (unit == 'M') {
			sizeValue *= 1024 * 1024; // Mega
		} else if (unit == 'G') {
			sizeValue *= 1024 * 1024 * 1024; // Giga
		}

		// Check that the size does not exceed 10G
		const long long max_size = 10LL * 1024 * 1024 * 1024; // 10G in bytes
		if (sizeValue > max_size) {
			throw std::runtime_error(ERR_MAX_SIZE_RANGE(dirValue));
		}

		std::cout << "Valid size: " << dirValue << " (in bytes: " << sizeValue << ")" << std::endl;
	}

void Parser::_checkServerN(std::string& dirValue) {
		// Remove the ending semicolon
		this->_evalDelEndSemiColon(dirValue);

		// Check that the server name is valid (alphanumeric, hyphens, and periods)
		if (dirValue.empty()) {
			throw std::runtime_error(ERR_INVALID_SERVER_NAME(dirValue));
		}

		for (char c : dirValue) {
			// Check for valid characters
			if (!std::isalnum(c) && c != '-' && c != '.') {
				throw std::runtime_error(ERR_INVALID_SERVER_NAME(dirValue));
			}
		}
}

void Parser::_checkErrorP(std::string& dirValue) {
		// delete ending ';' if necessary to get a cleaner string later
		this->_evalDelEndSemiColon(dirValue);

		// find 'root' path in the map
		auto it = _tempServerConfigMap.find(ERROR_P);
		if (it == _tempServerConfigMap.end()) {
			throw std::runtime_error(ERR_DIRECTIVE_MISSING(ERROR_P));
		}
		
		// Get path
		std::string rootDir = it->second; 

		// make fullPath to the file
		std::string fullPath = rootDir + "/" + dirValue;

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

	std::string const mandatoryParam[] = {LOCATION};
	//forbiden = Location parameter
	std::string const forbiddenParam[] = {LISTEN, HOST, ROOT, INDEX, MAX_SIZE, SERVER_N, ERROR_P};

	for (int i = 0; i < 1; i++)
	{
		if (_tempServerConfigMap.find(mandatoryParam[i]) == _tempServerConfigMap.end())
			throw std::runtime_error(ERR_DIRECTIVE_MISSING(mandatoryParam[i]));
	}

	for (int i = 0; i < 7; i++)
	{
		if (_tempServerConfigMap.find(forbiddenParam[i]) != _tempServerConfigMap.end())
			throw std::runtime_error(ERR_FORBIDDEN_DIRECTIVE(forbiddenParam[i]));
	}

}

void	Parser::_checkLocDirValue(){

	for (std::map<std::string, std::string>::iterator it = _tempLocationConfigMap.begin(); it != _tempLocationConfigMap.end(); ++it) {
		if (it->second == LOCATION) {
			this->_checkLocation(it->second);
		}
		else if (it->second == ALLOW_M) {
			this->_checkAllowM(it->second);
		}
		else if (it->second == TRY) {
			this->_checkTry(it->second);
		}
		else if (it->second == RETURN) {
			this->_checkReturn(it->second);
		}
		else if (it->second == AUTOID) {
			this->_checkAutoID(it->second);
		}
		else if (it->second == ROOT_LOC) {
			this->_checkRootLoc(it->second);
		}
		else if (it->second == UPLOAD) {
			this->_checkUpload(it->second);
		}
		else if (it->second == CGI_P) {
			this->_checkCgiP(it->second);
		}
		else if (it->second == CGI_E) {
			this->_checkCgiE(it->second);
		}
	}
	

}


//------ Directives' Location checking functions:
void Parser::_checkLocation(std::string& dirValue) {
		if (dirValue.empty() || dirValue[0] != '/') {
			throw std::runtime_error(ERR_LOCATION(dirValue));
		}
}

void Parser::_checkAllowM(std::string& dirValue) {
	// delete ending ';' if necessary to get a cleaner string later
		this->_evalDelEndSemiColon(dirValue);

	// Set of allowed methods
	std::set<std::string> allowedMethods = {"GET", "POST", "DELETE"}; //"PUT", "PATCH", "OPTIONS", "HEAD"

	// Split the methods by space and check validity
	std::stringstream ss(dirValue);
	std::string method;
	while (ss >> method) {
		if (allowedMethods.find(method) == allowedMethods.end()) {
			throw std::runtime_error(ERR_INVALID_METHOD(method, dirValue));
		}
	}

}

void Parser::_checkLocation(std::string& dirValue) {
		if (dirValue.empty() || dirValue[0] != '/') {
			throw std::runtime_error(ERR_LOCATION(dirValue));
		}
}




void Parser::_evalDelEndSemiColon(std::string& s){
	if (s.back() != ';')
			throw std::runtime_error(ERR_SEMICOLON(this->_nbLine, line));

		// delete last character (;)
		if (!s.empty() && s.back() == ';')
				s.pop_back();
}









void Parser::_getParamFromToken(int enumToken){
	std::string line = this->_tokensVector[this->_nbLine];
	std::istringstream iss(line);
	std::string dirName, dirValue;
	
	//!!! There is at least a space between parameters. Ex: "location_space_.py_space_{"

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

	//No more checks here.	

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