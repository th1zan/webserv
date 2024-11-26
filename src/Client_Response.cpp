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
    send(_socket, response.str().c_str(), response.str().size(), 0);
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
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " Found\r\n";
    response << "Location: " << location << "\r\n";
    response << "Content-Length: 0\r\n\r\n";
    send(_socket, response.str().c_str(), response.str().size(), 0);
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

// TODO: Add custom error pages - 400, 403, 404, 405, 411, 413, 415, 500, 501
// log error message?
// maybe make a map for all those error codesvto avoid so many if else
void Client::sendErrorResponse(int statusCode, const std::string &statusMessage)
{
    std::string customErrorPagePath;
    std::ifstream errorPageFile; 
    std::string errorPageContent;

    // Map of status codes to error page paths
    std::map<int, std::string> errorPagePaths;
    errorPagePaths[400] = "error_pages/400.html";
    errorPagePaths[403] = "error_pages/403.html";
    errorPagePaths[404] = "error_pages/404.html";
    errorPagePaths[405] = "error_pages/405.html";
    errorPagePaths[411] = "error_pages/411.html";
    errorPagePaths[413] = "error_pages/413.html";
    errorPagePaths[415] = "error_pages/415.html";
    errorPagePaths[500] = "error_pages/500.html";
    errorPagePaths[501] = "error_pages/501.html";
    
    // Find the custom error page path or use a default
    std::map<int, std::string>::iterator it = errorPagePaths.find(statusCode);
    if (it != errorPagePaths.end())
        customErrorPagePath = this->_server.getRoot() + it->second;
    else
        customErrorPagePath = this->_server.getRoot() + "error_pages/error.html";
      
    // // Handle specific custom error pages
    // if (statusCode == 400)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/400.html";
    // else if (statusCode == 403)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/403.html";
    // else if (statusCode == 404)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/404.html";
    // else if (statusCode == 405)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/405.html";
    // else if (statusCode == 411)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/411.html";
    // else if (statusCode == 413)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/413.html";
    // else if (statusCode == 415)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/415.html";
    // else if (statusCode == 500)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/500.html";
    // else if (statusCode == 501)
    //     customErrorPagePath = this->_server.getRoot() + "error_pages/501.html";
	// else
	// 	customErrorPagePath = this->_server.getRoot() + "error_pages/error.html";

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
    sendResponse(statusCode, statusMessage, defaultErrorBody.str());
}
