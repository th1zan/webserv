/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 13:39:20 by zsoltani          #+#    #+#             */
/*   Updated: 2024/10/24 16:42:34 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <sys/socket.h>   // For send() 
#include <iostream> // for logging output
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>  // For close(), write(), and remove()

class Request {
public:
    Request(const std::map<std::string, std::string>& config);
    void processRequest(int clientSocket);

private:
    void get(int clientSocket, const std::string& path);
    void post(int clientSocket, const std::string& path);
    void del(int clientSocket, const std::string& path);

    std::map<std::string, std::string> configSettings;
};

#endif

