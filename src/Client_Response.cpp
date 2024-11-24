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
 * @details Handles the following:
 * - Attempts to load a custom error page based on the status code and server root directory.
 * - Sends the custom error page if available; otherwise, sends a default HTML error message.
 * 
 * @note Custom error pages are searched for the following status codes:
 * - 404: Not Found (`404.html`)
 * - 500: Internal Server Error (`500.html`)
 * - Other errors: General error page (`error.html`)
 * 
 * @param [in] statusCode The HTTP status code indicating the error (e.g., 404, 500).
 * @param [in] statusMessage The HTTP status message describing the error.
 */

// TODO: Add custom error pages
// log error message?
void Client::sendErrorResponse(int statusCode, const std::string &statusMessage)
{
    std::string customErrorPagePath;
    std::ifstream errorPageFile;
    std::string errorPageContent;

    // Handle specific custom error pages
    if (statusCode == 404)
        customErrorPagePath = this->_server.getRoot() + "/404.html";
    else if (statusCode == 500)
        customErrorPagePath = this->_server.getRoot() + "/500.html";
	// others error codes?
	else
		customErrorPagePath = this->_server.getRoot() + "/error.html";
    if (!customErrorPagePath.empty())
    {
        errorPageFile.open(customErrorPagePath.c_str());
        if (errorPageFile.is_open())
        {
            // Read custom error page content
            errorPageContent = std::string((std::istreambuf_iterator<char>(errorPageFile)), std::istreambuf_iterator<char>());
            errorPageFile.close();
            sendResponse(statusCode, statusMessage, errorPageContent);
            return;
        }
    }
    // Fallback for other status codes or missing custom error pages
    std::ostringstream defaultErrorBody;
    defaultErrorBody << "<html><body><h1>" << statusCode << " " << statusMessage << "</h1></body></html>";
    sendResponse(statusCode, statusMessage, defaultErrorBody.str());
}
