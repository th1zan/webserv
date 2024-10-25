/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 16:05:47 by zsoltani          #+#    #+#             */
/*   Updated: 2024/10/24 16:43:11 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

WebServer::WebServer() : serverSocket(-1)
{
    std::cout << "WebServer created." << std::endl;
}

WebServer::~WebServer()
{
    if (serverSocket != -1)
        close(serverSocket); 
    std::cout << "WebServer destroyed." << std::endl;
}

void WebServer::startServer(const std::string& configFile)
{
    parseConfigFile(configFile);
    setupSocket();

    std::map<std::string, std::string>::const_iterator it;
    for (it = configSettings.begin(); it != configSettings.end(); ++it)
    {
        std::cout << it->first << " = " << it->second << std::endl;
    }

    while (true)
    {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == -1)
        {
            std::cerr << "Error: Failed to accept connection." << std::endl;
            continue;
        }

        handleClientRequest(clientSocket);

        close(clientSocket);
    }
}

void WebServer::stopServer()
{
    std::cout << "Stopping server." << std::endl;
    if (serverSocket != -1)
    {
        close(serverSocket);
        serverSocket = -1;
    }
}

void WebServer::setupSocket()
{
    int port = std::stoi(configSettings["port"]);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        std::cerr << "Error: Failed to create socket." << std::endl;
        exit(1);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Error: Failed to set socket options." << std::endl;
        close(serverSocket);
        exit(1);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error: Failed to bind socket to port " << port << std::endl;
        close(serverSocket);
        exit(1);
    }

    if (listen(serverSocket, 10) == -1)
    {
        std::cerr << "Error: Failed to listen on socket." << std::endl;
        close(serverSocket);
        exit(1);
    }

    std::cout << "Server listening on port " << port << std::endl;
}

void WebServer::handleClientRequest(int clientSocket)
{
    char buffer[1024] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);

    if (bytesRead < 0)
    {
        std::cerr << "Error reading request from client." << std::endl;
        return;
    }

    std::istringstream requestStream(buffer);
    std::string method, path, version;
    requestStream >> method >> path >> version;
    
    // GET request
    if (method == "GET")
    {
        std::string filePath = configSettings["root"] + path;
        if (path == "/")
            filePath = configSettings["root"] + "/index.html";

        std::ifstream file(filePath);
        if (file.is_open())
        {
            std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Length: " << fileContents.size() << "\r\n";
            response << "\r\n";
            response << fileContents;

            send(clientSocket, response.str().c_str(), response.str().size(), 0);
        }
        else
        {
            const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
            send(clientSocket, notFoundResponse, strlen(notFoundResponse), 0);
        }
    }

    // POST request
    else if (method == "POST")
    {
        const char* postResponse = "HTTP/1.1 200 OK\r\nContent-Length: 7\r\n\r\nPOST OK";
        send(clientSocket, postResponse, strlen(postResponse), 0);
    }

    // DELETE request
    else if (method == "DELETE")
    {
        std::string filePath = configSettings["root"] + path;

        if (std::remove(filePath.c_str()) == 0)
        {
            const char* deleteResponse = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nFile Deleted";
            send(clientSocket, deleteResponse, strlen(deleteResponse), 0);
        }
        else
        {
            const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
            send(clientSocket, notFoundResponse, strlen(notFoundResponse), 0);
        }
    }
    // any other methods not implemented
    else
    {
        const char* notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 19\r\n\r\n501 Not Implemented";
        send(clientSocket, notImplementedResponse, strlen(notImplementedResponse), 0);
    }
}

void WebServer::parseConfigFile(const std::string& configFile)
{
    std::ifstream file(configFile);
    if (!file)
    {
        std::cerr << "Error: Could not open configuration file: " << configFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value))
            configSettings[key] = value;
    }
    std::cout << "Finished parsing config file." << std::endl;
}
