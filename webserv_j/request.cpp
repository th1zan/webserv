/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 13:38:37 by zsoltani          #+#    #+#             */
/*   Updated: 2024/10/24 16:43:37 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "request.hpp"

Request::Request(const std::map<std::string, std::string>& config)
    : configSettings(config) {}

void Request::processRequest(int clientSocket)
{
    char buffer[1024] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);

    if (bytesRead < 0)
	{
        std::cerr << "Error reading request from client." << std::endl;
        return;
    }

// Print the entire raw HTTP request to debug
    std::cout << "Raw HTTP Request: \n" << buffer << std::endl;

    std::istringstream requestStream(buffer);
    std::string method, path, version;
    requestStream >> method >> path >> version;

    // Print the parsed method, path, and version to verify correctness
    std::cout << "Parsed Method: " << method << ", Path: " << path << ", Version: " << version << std::endl;

    if (method == "GET")
        get(clientSocket, path);
    else if (method == "POST")
        post(clientSocket, path);
    else if (method == "DELETE")
        del(clientSocket, path);
    else
	{
        const char* notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 19\r\n\r\n501 Not Implemented";
        send(clientSocket, notImplementedResponse, strlen(notImplementedResponse), 0);
    }
}

void Request::get(int clientSocket, const std::string& path)
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

void Request::post(int clientSocket, const std::string& path)
{
    (void)path;
    const char* postResponse = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nPOST OK";
    send(clientSocket, postResponse, strlen(postResponse), 0);
}

void Request::del(int clientSocket, const std::string& path)
{
    std::string filePath = configSettings["root"] + path;

    if (std::remove(filePath.c_str()) == 0)
	{
        const char* deleteResponse = "HTTP/1.1 200 OK\r\nContent-Length: 15\r\n\r\nFile Deleted";
        send(clientSocket, deleteResponse, strlen(deleteResponse), 0);
    }
	else
	{
        const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
        send(clientSocket, notFoundResponse, strlen(notFoundResponse), 0);
    }
}
