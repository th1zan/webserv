#include "Server.hpp"

Server::Server(){

	this->_checkKeywords(configs);

	this->_serverName = this->_getValidName(servers, configs[SERVER_N]);
	this->_host = configs[HOST];
	this->_root = this->_getValidRoot(configs[ROOT]);
	this->_index = this->_checkAndGetPage(configs[INDEX]);
	this->_errorPage = this->_checkAndGetPage(configs[ERROR_P]);
	this->_port = this->_getValidPort(configs[LISTEN]);
	this->_clientMaxBodySize = this->_getConvertedMaxSize(configs[MAX_SIZE]);
	this->_errorResponse = this->_generateErrorResponse();
	this->_isDefault = this->_checkDefaultServer(servers);
	this->_socket = 0;
}



Server::~Server(){}