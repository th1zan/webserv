/**
 * @file Parser.hpp
 * @brief Header file for the Parser class.
 *
 * This file contains the declaration of the Parser class, which is responsible
 * for parsing server configuration files and initializing server instances.
 */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "defines.hpp"
#include "Server.hpp"
#include "utils.hpp"

/**
 * @enum eToken
 * @brief Enumeration of token types for the parser.
 *
 * This enumeration defines the various types of tokens that can be recognized
 * during the parsing process.
 */
enum eToken {
	TK_SERVER,
	TK_LOCATION,
	TK_CLOSE_BRACKET,
	TK_TOKEN,
	TK_ERROR,
	TK_EMPTY,
	TK_COMMENT
};

/**
 * @struct Token
 * @brief Represents a parsed token.
 *
 * This structure holds a token's type and its associated value.
 */
struct Token
{
	eToken type;
	std::string value;
};

typedef std::vector<Server> serverVector;
class Server;

/**
 * @class Parser
 * @brief Parses server configuration files and initializes server instances.
 *
 * The Parser class processes input arguments, reads configuration files,
 * and initializes server instances based on the parsed data.
 */
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
			void	_delEndSemiColon(std::string& s);
			void	_checkPath(std::string& dirValue, bool isDir, bool hasWPer = false);

			//Directives checking functions:
			void	_checkListen(std::string& dirValue);
			
			void	_checkHost(std::string& dirValue);
			bool	_isValidIpAddress(const std::string& str);
			bool	_isValidHostName(const std::string& str);
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

			size_t												_nServer; ///< Number of servers parsed.
			size_t												_nbLine; ///< Current line number in the configuration file.
			std::string											_confFilePath; ///< Path to the configuration file.
			std::ifstream										_confFile; ///< Input file stream for the configuration file.
			std::vector<Token>										_tokensVector; ///< Vector of parsed tokens.
			std::vector<Server>									_serversVector; ///< Vector of Server instances.
			std::map<std::string, std::string>					_tempServerConfigMap; ///< Temporary server configuration map.
			std::vector<std::map<std::string, std::string> >	_tempLocationMapVector; ///< Vector of temporary location configuration maps.
			std::map<std::string, std::string>					_tempLocationConfigMap; ///< Temporary location configuration map.
			std::vector<location_t>								_tempLocationVector; ///< Vector of temporary locations.
			location_t											_tempLocation; ///< Temporary location structure.
			std::string											_tempRootDirPath; ///< Temporary root directory path.
			
	public:
			Parser(int argc, char **argv);
			~Parser();
			std::vector<Server>& getServersVector();
			std::set<std::string> getSupportedExtensions();
};
#endif
