/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client_Response.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 16:09:12 by zsoltani          #+#    #+#             */
/*   Updated: 2024/12/05 02:32:58 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

// // Send responses
// void Client::sendResponse(int statusCode, const std::string &statusMessage, const std::string &body) { /* ... */ }
// void Client::sendRedirectResponse(int statusCode, const std::string &location) { /* ... */ }
// void Client::sendErrorResponse(int statusCode, const std::string &statusMessage) { /* ... */ }


/**
 * @brief Sends a standard HTTP response to the client.
 * 
 * @details Constructs and sends an HTTP response with the given status code, status message, 
 * and body. Automatically sets the `Content-Length` header based on the body size.
 * 
 * @param [in] statusCode The HTTP status code (e.g., 200 for OK, 404 for Not Found).
 * @param [in] statusMessage The HTTP status message (e.g., "OK", "Not Found").
 * @param [in] body The response body as a string.
 */
void Client::sendResponse(int statusCode, const std::string &statusMessage, const std::string &body)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    //send(_socket, response.str().c_str(), response.str().size(), 0);

    ssize_t bytesSent = send(_socket, response.str().c_str(), response.str().size(), 0);
    if (bytesSent < 0) {
        throw std::runtime_error("Send failed in sendRedirectResponse");
    }

}

/**
 * @brief Sends an HTTP redirect response to the client.
 * 
 * @details Constructs and sends an HTTP response with the given status code and location.
 * The client will be redirected to the specified location.
 * 
 * @param [in] statusCode The HTTP status code (e.g., 301 for Moved Permanently, 302 for Found).
 * @param [in] location The URL to redirect the client to.
 */
void Client::sendRedirectResponse(int statusCode, const std::string &location)
{
    std::cerr << "Ressource location found: " << statusCode << "  " << location << std::endl;
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " Found\r\n";
    response << "Location: " << location << "\r\n";
    response << "Content-Length: 0\r\n\r\n";
    
    //send(_socket, response.str().c_str(), response.str().size(), 0);

    ssize_t bytesSent = send(_socket, response.str().c_str(), response.str().size(), 0);
    if (bytesSent < 0) {
        throw std::runtime_error("Send failed in sendRedirectResponse");
    }
}

/**
 * @brief Sends an error response to the client.
 * 
 * @details Handles error responses by:
 * - Mapping HTTP status codes to custom error page file paths.
 * - Attempting to load the custom error page based on the status code and server root directory.
 * - Sending the custom error page if it exists and is accessible.
 * - Falling back to a default HTML error message if the custom page is unavailable.
 * 
 * @note Supported status codes with custom error pages include:
 * - 400: Bad Request (`400.html`)
 * - 403: Forbidden (`403.html`)
 * - 404: Not Found (`404.html`)
 * - 405: Method Not Allowed (`405.html`)
 * - 411: Length Required (`411.html`)
 * - 413: Payload Too Large (`413.html`)
 * - 415: Unsupported Media Type (`415.html`)
 * - 500: Internal Server Error (`500.html`)
 * - 501: Not Implemented (`501.html`)
 * For unsupported or missing status codes, a general error page (`error.html`) is used.
 * 
 * @param [in] statusCode The HTTP status code indicating the error (e.g., 404, 500).
 * @param [in] statusMessage The HTTP status message describing the error.
 */
void Client::sendErrorResponse(int statusCode, const std::string &statusMessage)
{
    std::string customErrorPagePath;
    std::ifstream errorPageFile; 
    std::string errorPageContent;

    // Map of status codes to error page paths
    std::map<int, std::string> errorPagePaths;
    errorPagePaths[400] = "/error_pages/400.html";
    errorPagePaths[403] = "/error_pages/403.html";
    errorPagePaths[404] = "/error_pages/404.html";
    errorPagePaths[405] = "/error_pages/405.html";
    errorPagePaths[411] = "/error_pages/411.html";
    errorPagePaths[413] = "/error_pages/413.html";
    errorPagePaths[415] = "/error_pages/415.html";
    errorPagePaths[500] = "/error_pages/500.html";
    errorPagePaths[501] = "/error_pages/501.html";
    
    // Find the custom error page path or use a default
    std::map<int, std::string>::iterator it = errorPagePaths.find(statusCode);
    if (it != errorPagePaths.end())
        customErrorPagePath = this->_server.getRoot() + it->second;
    else
        customErrorPagePath = this->_server.getRoot() + "/error_pages/error.html";

    errorPageFile.open(customErrorPagePath.c_str());
    if (errorPageFile.is_open())
    {
        // Read custom error page content
        errorPageContent = std::string((std::istreambuf_iterator<char>(errorPageFile)), std::istreambuf_iterator<char>());
        errorPageFile.close();
        sendResponse(statusCode, statusMessage, errorPageContent);
        return;
    }
    // Fallback for other status codes or missing custom error pages
    std::ostringstream defaultErrorBody;
    defaultErrorBody << "<html><body><h1>" << statusCode << " " << statusMessage << "</h1></body></html>";
    // Debug: Sending fallback response
    std::cout << "Debug: Sending fallback error response for status code: " << statusCode << std::endl;
    sendResponse(statusCode, statusMessage, defaultErrorBody.str());
} 

void Client::sendCgiResponse(const std::string &cgiOutput)
{
    // Find the end of CGI headers (if present)
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    std::string body;
    std::string contentType = "text/html"; // Default content type

    if (headerEnd != std::string::npos)
    {
        // Extract headers and body
        std::string headers = cgiOutput.substr(0, headerEnd);
        body = cgiOutput.substr(headerEnd + 4);

        // Parse headers to determine Content-Type
        size_t contentTypePos = headers.find("Content-type:");
        if (contentTypePos != std::string::npos)
        {
            size_t start = contentTypePos + 13; // Skip "Content-type:"
            size_t end = headers.find("\r\n", start);
            contentType = headers.substr(start, end - start);
            stringTrim(contentType); // Trim whitespace
        }
    }
    else
    {
        // If no headers, treat the entire output as the body
        body = cgiOutput;
    }

    // Send the response with the parsed Content-Type
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n\r\n";
    response << body;

    //send(_socket, response.str().c_str(), response.str().size(), 0);
   
    ssize_t bytesSent = send(_socket, response.str().c_str(), response.str().size(), 0);
    if (bytesSent < 0) {
        throw std::runtime_error("Send failed in sendRedirectResponse");
    }
    
}
