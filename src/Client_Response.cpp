#include "Client.hpp"

// // Send responses
// void Client::sendResponse(int statusCode, const std::string &statusMessage, const std::string &body) { /* ... */ }
// void Client::sendRedirectResponse(int statusCode, const std::string &location) { /* ... */ }
// void Client::sendErrorResponse(int statusCode, const std::string &statusMessage) { /* ... */ }


// Send a response
void Client::sendResponse(int statusCode, const std::string &statusMessage, const std::string &body)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    send(_socket, response.str().c_str(), response.str().size(), 0);
}

void Client::sendRedirectResponse(int statusCode, const std::string &location)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " Found\r\n";
    response << "Location: " << location << "\r\n";
    response << "Content-Length: 0\r\n\r\n";
    send(_socket, response.str().c_str(), response.str().size(), 0);
}

// Send an error response
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
