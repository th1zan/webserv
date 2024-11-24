#include "Client.hpp"

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
    return locationConfig.hasCGI && filePath.find(locationConfig.cgiExtension) != std::string::npos;
}

/**
 * @brief Executes a CGI (Common Gateway Interface) script.
 * 
 * @details This function is responsible for executing CGI scripts based on the server's configuration and client requests. 
 * The function is currently a placeholder and sends a 501 Not Implemented response.
 * 
 * Planned functionality:
 * - Fork a process to execute the CGI script.
 * - Use `execve()` to run the script in the child process.
 * - Capture the script's output (`stdout`) using pipes and send it as the HTTP response.
 * - Handle errors and unexpected behavior during execution.
 * 
 * @param [in] cgiPath The path to the CGI executable or interpreter (e.g., `/usr/bin/python`).
 * @param [in] filePath The path to the script or file to be executed by the CGI interpreter.
 */
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
