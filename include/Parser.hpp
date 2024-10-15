	#ifndef PARSER_HPP
#define PARSER



#include "defines.hpp"
#include "Server.hpp"
#include "utils.hpp"

enum eToken {
	TK_SERVER,
	TK_LOCATION,
	TK_CLOSE_BRACKET,
	TK_TOKEN,
	TK_ERROR,
	TK_EMPTY,
	TK_COMMENT
};


struct Token
{
	eToken type;
	std::string value;
};

class Parser{

	private:
			void	_checkInputArg(int argc, char **argv);
			void	_parseFile();
			void	_checkBracket();
			void	_getConfigAndInitServers();
			void	_getConfigFromTokens();
			void	_getParamFromToken(int enumToken);
			void	_getServerParam();
			void	_checkServerParam();
			void	_getLocationParam();
			void	_checkLocationParam();
			void	_checkDirectiveName();
			void	_checkDirectiveValue();
			void	_checkLocDirName();
			void	_checkLocDirValue();
			void	_checkConfigs();
			void	_evalDelEndSemiColon(std::string& s);
			void	_checkPath(std::string& dirValue, bool isDir, bool hasWPerm);

			//Directives checking functions:
			void	_checkListen(std::string& dirValue);
			
			void	_checkHost(std::string& dirValue);
			void	_checkRoot(std::string& dirValue);
			void	_checkIndex(std::string& dirValue);
			void	_checkMaxSize(std::string& dirValue);
			void	_checkServerN(std::string& dirValue);
			void	_checkErrorP(std::string& dirValue);

			//Directives' Location checking functions:
			void	_checkLocation(std::string& dirValue);
			void	_checkAllowM(std::string& dirValue);
			void	_checkTry(std::string& dirValue);
			void	_checkReturn(std::string& dirValue);
			void	_checkAutoID(std::string& dirValue);
			void	_checkRootLoc(std::string& dirValue);
			void	_checkUpload(std::string& dirValue);
			void	_checkCgiP(std::string& dirValue);
			void	_checkCgiE(std::string& dirValue);

			

			size_t		_nServer;
			size_t		_nbLine;


			std::string							_confFilePath;
			std::ifstream						_confFile;
			std::vector<Token>					_tokensVector;

			std::vector<Server>					_serversVector;
			std::map<std::string, std::string>	_tempServerConfigMap;

			std::vector<std::map<std::string, std::string>>		_tempLocationMapVector;
			std::map<std::string, std::string>					_tempLocationConfigMap;
			std::vector<Server::location_t>						_tempLocationVector;
			location_t											_tempLocation;
			


	public:
			Parser(int argc, char **argv);
			~Parser();
			std::vector<Server>& getServersVector();

			


};

#endif