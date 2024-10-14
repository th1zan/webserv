#ifndef SERVER_HPP
#define SERVER_HPP

typedef struct location_s
{
	std::string					root;
	std::vector<std::string>	methods;
	std::string					redirect;
	bool						autoindex;
	std::string					tryFile;
	bool						hasCGI;
	std::string					cgiPath;
	std::string					cgiExtension;
	std::string					uploadTo;
}					location_t;

class Server{
	private:
			std::string 		_serverName;
			std::string 		_port;
			std::string 		_host;
			std::string 		_root;
			std::string 		_index;
			size_t				_clientMaxBodySize;
			std::string 		_errorPage;
			std::string 		_errorResponse;
			std::vector<location_t>	_tempLocationVector;
			bool				_isDefault;
			int					_socket;


	public:
		

		///verifier que les noms de servers ne sont pas les memes !

};

#endif