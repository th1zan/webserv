#include "Client.hpp"

// Check if the path is a CGI script
// bool Client::isCgiPath(const std::string& filePath, const location_t& locationConfig) const
// {
//     return locationConfig.hasCGI && filePath.find(locationConfig.cgiExtension) != std::string::npos;
// }

// Placeholder for CGI execution Joshua's function
void Client::executeCgi(const std::string &cgiPath, const std::string &filePath) 
{
    //(void)cgiPath;
    //(void)filePath;
    // Log the attempt to execute a CGI script
    std::cout << "Attempted to execute CGI script: " << cgiPath + "/" + filePath << std::endl;
    sendErrorResponse(501, "CGI Execution Not Implemented");
    // Fork a process.
	// Use execve() to execute the script.
	// Capture the script’s output (stdout) using pipes.
}
