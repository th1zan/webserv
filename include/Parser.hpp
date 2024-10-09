#ifndef PARSER_HPP
#define PARSER



#include "defines.hpp"
#include "Server.hpp"
#include "utils.hpp"

enum eToken {
	SERVER,
	LOCATION,
	CLOSE_BRACKET,
	TOKEN,
	ERROR,
	EMPTY,
	COMMENT
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
			void	_checkTokens();
			void	_getServerConfigFromTokens();
			void	_checkConfigs();

			std::string			_confFilePath;
			std::ifstream		_confFile;
			std::vector<Token>	_tokensVector;
			std::vector<Server>	_serversVector;
			std::map<std::string, std::string>	_configMap;



	public:
			Parser(int argc, char **argv);
			~Parser();

			


};

#endif