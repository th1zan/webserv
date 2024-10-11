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
			void	_checkBracket()
			void	_getConfigAndInitServers();
			void	_getConfigFromTokens();
			void	_getParamFromToken(int enumToken);
			void	_getServerParam();
			void	_checkServerParam();
			void	_getLocationParam();
			void	_checkLocationParam();
			void	_checkConfigs();
			size_t		_nServer;
			size_t		_nbLine;


			std::string							_confFilePath;
			std::ifstream						_confFile;
			std::vector<Token>					_tokensVector;
			std::vector<Server>					_serversVector;
			std::map<std::string, std::string>	_tempServerConfigMap;
			std::vector<Server::location_t>		_tempLocationVector;
			std::map<std::string, std::string>	_tempLocationConfigMap;
			location_t							_tempLocation;


	public:
			Parser(int argc, char **argv);
			~Parser();

			


};

#endif