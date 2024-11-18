#include "Client.hpp"

// Check if the path is a CGI script
bool Client::isCgiPath(const std::string& filePath, const location_t& locationConfig) const
{
    return locationConfig.hasCGI && filePath.find(locationConfig.cgiExtension) != std::string::npos;
}

// Placeholder for CGI execution Joshua's function
void Client::executeCgi(const std::string &cgiPath, const std::string &filePath) 
{
    (void)cgiPath;
    (void)filePath;
    sendErrorResponse(501, "CGI Execution Not Implemented");
}
