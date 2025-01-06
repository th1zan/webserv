/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client_CGI.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 16:06:51 by zsoltani          #+#    #+#             */
/*   Updated: 2024/12/05 16:07:15 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Client.hpp"
#include "cgi.hpp"
/**
 * @brief Checks if the given file path corresponds to a CGI script.
 * 
 * @details This function determines whether a file path matches the criteria for a CGI script,
 * based on the server's location configuration. The check verifies:
 * - Whether CGI is enabled (`hasCGI`) for the location.
 * - If the file path contains the configured CGI extension.
 * 
 * @param [in] filePath The path to the file being checked.
 * @param [in] locationConfig The location configuration containing CGI settings.
 * @return true if the file path is identified as a CGI script, false otherwise.
 */

bool Client::isCgiPath(const std::string& filePath, const location_t& locationConfig) const
{
    std::cout << "Checking CGI path: " << filePath << std::endl;
    std::cout << "Has CGI: " << locationConfig.hasCGI << ", CGI Extension: " << locationConfig.cgiExtension << std::endl;
    // Ensure CGI is enabled for the location
    if (!locationConfig.hasCGI)
    {
        std::cout << "CGI is not enabled for this location." << std::endl;
        return false;
    }

    // Check if the file path ends with the CGI extension
    if (filePath.size() >= locationConfig.cgiExtension.size() &&
        filePath.compare(filePath.size() - locationConfig.cgiExtension.size(), locationConfig.cgiExtension.size(), locationConfig.cgiExtension) == 0) {
        return true;
    }

    std::cout << "Path does not match CGI extension: " << locationConfig.cgiExtension << std::endl;
    return false;
}

/**
 * @brief 
 * 
 * @param cgiPath 
 * @param method 
 * @param queryString 
 * @param body 
 * @param pathInfo 
 * @param scriptFileName 
 * @return std::string 
 */

std::string Client::executeCgi(const std::string &cgiPath, const std::string &method, 
                        const std::string &queryString, const std::string &body, 
                        const std::string &pathInfo, const std::string &scriptFileName)
{
    CGI cgi(cgiPath, method, queryString, body, pathInfo, scriptFileName);
    
     // Debug CGI execution details
    std::cout << "Debug: Executing CGI script with:" << std::endl;
    std::cout << "Debug: cgiPath: " << cgiPath << std::endl;
    std::cout << "Debug: method: " << method << std::endl;
    std::cout << "Debug: queryString: " << queryString << std::endl;
    std::cout << "Debug: pathInfo: " << pathInfo << std::endl;
    std::cout << "Debug: scriptFileName: " << scriptFileName << std::endl;
    std::string cgiOutput = cgi.execute(); // Execute CGI and capture the output.
    return cgiOutput; // Return the output to the caller.
}
