#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>   // For file handling
#include <cstring>   // For memset
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>  // For close()
#include <sstream>   // For creating HTTP responses

class WebServer {
public:
    WebServer();
    ~WebServer();

    void startServer(const std::string& configFile);
    void stopServer(); 
private:
    void parseConfigFile(const std::string& configFile); 
    void setupSocket(); // listening socket
    void handleClientRequest(int clientSocket); // HTTP requests and serve static files

    
    std::map<std::string, std::string> configSettings;   // config key-value pairs
    
    int serverSocket; // fd for the server's listening socket
};

#endif
