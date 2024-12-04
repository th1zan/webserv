/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client_File.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 16:08:42 by zsoltani          #+#    #+#             */
/*   Updated: 2024/12/04 20:48:19 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

// list of functions
// // Handle GET requests
// void Client::handleGetRequest(const std::string& path)
// std::string Client::generateAutoindexPage(const std::string &directoryPath, const std::string &requestPath)

// // Handle POST requests
// void Client::handlePostRequest(const std::string &path)
// void Client::handleMultipartFormData(const std::string &path, const std::string &boundary)
// void Client::uploadFile(const std::string &path)

// // Handle DELETE requests
// void Client::handleDeleteRequest(const std::string &path)
// bool Client::fileExists(const std::string& path) - should I move it to utils?


/**
 * @brief Handles a GET request to retrieve resources from the server.
 * 
 * @details This function:
 * - Resolves the requested file path.
 * - Serves the default `index.html` for root requests (`/`).
 * - Handles directory requests with autoindex generation if enabled.
 * - Sends the file content if the file exists, or an error response otherwise.
 * 
 * @param [in] path The requested resource path.
 */
void Client::handleGetRequest(const std::string& path)
{
    // Extract path without query string
    size_t queryPos = path.find('?');
    std::string cleanPath = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
    std::string queryString = (queryPos != std::string::npos) ? path.substr(queryPos + 1) : "";
     // Debug the extracted clean path and query string
    std::cout << "Debug: Full path: " << path << std::endl;
    std::cout << "Debug: Clean path: " << cleanPath << std::endl;
    std::cout << "Debug: Query string: " << queryString << std::endl;

    // Construct the full file path
    std::string filePath = this->_server.getRoot() + cleanPath;

    // If the path is "/", serve the default index file
    if (cleanPath == "/")
        filePath = this->_server.getRoot() + "/index.html";
    
    // Check if the path is a CGI script
    location_t locationConfig = _server.getLocationConfig(cleanPath);

    // Check if the requested method is allowed in the location block
    for (std::vector<std::string>::iterator it = locationConfig.methods.begin(); it != locationConfig.methods.end(); ++it)
    {
        if (*it == "GET")
        {
            // The "GET" method is allowed, exit the loop.
            break;
        }

        // If we are at the last element and "GET" was not found, return an error.
        if (it + 1 == locationConfig.methods.end()) // Use iterator arithmetic since vector supports it.
        {
            sendErrorResponse(405, "Method Not Allowed");
            return;
        }
    }
    std::cout << "Debug: cleanPath: " << cleanPath << std::endl;
    std::cout << "Debug: CGI Path: " << locationConfig.cgiPath << std::endl;
    
    if (isCgiPath(cleanPath, locationConfig))
    {
        // Validate and calculate pathInfo
        // Calculate the script file name and path info
        //std::string pathInfo = cleanPath.substr(locationConfig.cgiPath.length());

        std::string pathInfo;
        if (cleanPath.find(locationConfig.cgiPath) == 0)
            pathInfo = cleanPath.substr(locationConfig.cgiPath.length());
        else
            {
                std::cerr << "Error: cleanPath does not start with cgiPath!" << std::endl;
                pathInfo = cleanPath; // Fallback for debugging
            }
            //std::cout << "Debug: pathInfo: " << pathInfo << std::endl;
            // Script file to be executed
            std::string scriptFileName = this->_server.getRoot() + cleanPath;
            //std::cout << "Debug: scriptFileName: " << scriptFileName << std::endl;

            // Python interpreter path
            std::string scriptPath = getPythonPath(); // Get the Python interpreter path

            // Execute CGI and capture output
            std::string result = executeCgi(scriptPath, "GET", queryString, "", pathInfo, scriptFileName);

            // Send the CGI output as the HTTP response
            if (!result.empty())
                sendCgiResponse(result);
            else
                sendErrorResponse(500, "Internal Server Error");
            return;
    }
    // autoindex
    // check if the path is a directory
    struct stat pathStat;
    if (stat(filePath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
    {
        // Check for index.html in the directory
        std::string indexPath = filePath + "/index.html";
        if (fileExists(indexPath))
            filePath = indexPath;
        else
        {
            // Check if autoindex is enabled
            location_t locationConfig = _server.getLocationConfig(cleanPath);
            if (locationConfig.autoindex)
            {
                // Generate the autoindex page
                std::string autoindexPage = generateAutoindexPage(filePath, cleanPath);
                sendResponse(200, "OK", autoindexPage);
                return;
            }
            else
            {
                sendErrorResponse(403, "Forbidden: Directory listing not allowed");
                return;
            }
        }
    }
    // Attempt to open the file
    std::ifstream file(filePath.c_str());
    if (file.is_open())
    {
        // Read file contents
        std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        // Send the file contents as the response
        sendResponse(200, "OK", fileContents);
    }
    else
        sendErrorResponse(404, "Not Found");
}


/**
 * @brief Generates an autoindex page for a directory.
 * 
 * @details Creates an HTML page listing the directory contents as hyperlinks. 
 * This function is invoked when autoindexing is enabled for the location.
 * 
 * @param [in] directoryPath The path to the directory on the server.
 * @param [in] requestPath The request path as seen by the client.
 * @return A string containing the autoindex HTML page.
 */
std::string Client::generateAutoindexPage(const std::string &directoryPath, const std::string &requestPath)
{
    std::ostringstream autoindex;
    autoindex << "<html><body><h1>Index of " << requestPath << "</h1><ul>";

    DIR *dir = opendir(directoryPath.c_str());
    if (!dir)
    {
        sendErrorResponse(500, "Internal Server Error: Cannot open directory");
        return "";
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == ".") continue; // Skip current directory
        autoindex << "<li><a href=\"" << requestPath;
        if (back(requestPath) != '/') autoindex << '/';
        autoindex << name << "\">" << name << "</a></li>";
    }
    closedir(dir);

    autoindex << "</ul></body></html>";
    return autoindex.str();
}

/**
 * @brief Handles a POST request to upload or process data on the server.
 * 
 * @details This function:
 * - Validates the Content-Type header.
 * - Handles multipart form data uploads or file uploads based on Content-Type.
 * - Executes CGI scripts if applicable.
 * 
 * @param [in] path The path where the resource will be uploaded or processed.
 */
void Client::handlePostRequest(const std::string &path)
 {
    // Extract path without query string
    size_t queryPos = path.find('?');
    std::string cleanPath = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;
    std::string queryString = (queryPos != std::string::npos) ? path.substr(queryPos + 1) : "";

    // std::cout << "Debug: Full path: " << path << std::endl;
    // std::cout << "Debug: Clean path: " << cleanPath << std::endl;
    // std::cout << "Debug: Query string: " << queryString << std::endl;
    
    // Retrieve location configuration for this request
    location_t locationConfig = _server.getLocationConfig(cleanPath);

    // Check if the requested method is allowed in the location block
    for (std::vector<std::string>::iterator it = locationConfig.methods.begin(); it != locationConfig.methods.end(); ++it)
    {
        if (*it == "POST")
        {
            // The "POST" method is allowed, exit the loop.
            break;
        }

        // If we are at the last element and "POST" was not found, return an error.
        if (it + 1 == locationConfig.methods.end()) // Use iterator arithmetic since vector supports it.
        {
            sendErrorResponse(405, "Method Not Allowed");
            return;
        }
    }
    // std::cout << "Debug: Location Config - CGI Path: " << locationConfig.cgiPath
    //           << ", Has CGI: " << locationConfig.hasCGI
    //           << ", CGI Extension: " << locationConfig.cgiExtension << std::endl;

    // Validate Content-Type header
    std::map<std::string, std::string>::iterator it = _headers.find("Content-Type");

    std::string contentType = (it != _headers.end()) ? it->second : "text/plain";
    stringTrim(contentType);

    // std::cout << "Normalized Content-Type: " << contentType << std::endl;
    
    if (isCgiPath(cleanPath, locationConfig))
    {
        // std::cout << "Debug: cleanPath: " << cleanPath << std::endl;
        // std::cout << "Debug: CGI Path: " << locationConfig.cgiPath << std::endl;

        // Determine PATH_INFO
        std::string pathInfo;
        if (cleanPath.find(locationConfig.cgiPath) == 0)
        {
            pathInfo = cleanPath.substr(locationConfig.cgiPath.length());
            // Correct behavior: Only keep path info beyond the script
            if (pathInfo.find('/') != 0) // Ensure it starts with a slash
            {
                pathInfo = ""; // Reset if no extra path info
            }
            else
            {
                pathInfo = ""; // No extra path info
            }
        }
        else
        {
            std::cerr << "Error: cleanPath does not start with cgiPath!" << std::endl;
            pathInfo = ""; // Default empty if no extra path info
        }

        // Script file to be executed
        std::string scriptFileName = this->_server.getRoot() + cleanPath;

        // Python interpreter path
        std::string scriptPath = getPythonPath(); // Get the Python interpreter path

        // std::cout << "Debug: scriptPath: " << scriptPath << std::endl;
        // std::cout << "Debug: scriptFileName: " << scriptFileName << std::endl;

        // executeCgi(scriptPath, "POST", queryString, _requestPayload, pathInfo, scriptFileName);
        std::string result = executeCgi(scriptPath, "POST", queryString, _requestPayload, pathInfo, scriptFileName);
        //std::cout << "Debug: CGI Result: " << result << std::endl;
        if (!result.empty())
            sendCgiResponse(result);
        else
            sendErrorResponse(500, "Internal Server Error");
        return;
    }
    // else
    // {
    //     std::cout << "Debug: Not a CGI script. Proceeding with standard POST handling." << std::endl; // Debugging
    // }
    // Handle file uploads if allowed
    if (!locationConfig.uploadTo.empty())
    {
        if (contentType == "application/json" || contentType == "text/plain" || contentType == "application/octet-stream" ||
            contentType == "application/pdf" || contentType.find("image/") == 0 || contentType == "application/x-www-form-urlencoded")
        {
            uploadFile(path);
            return;
        }
        // Check for multipart form data - maybe we'll remove if not needed
        else if (contentType.find("multipart/form-data") != std::string::npos)
        {
            size_t boundaryPos = contentType.find("boundary=");
            if (boundaryPos == std::string::npos)
            {
                sendErrorResponse(400, "Missing boundary parameter in Content-Type");
                return;
            }
            std::string boundary = contentType.substr(boundaryPos + 9); // Extract boundary
            handleMultipartFormData(path, boundary); // Handle multipart parsing
        }
        else {
            sendErrorResponse(415, "Unsupported Media Type for file upload");
            return;
        }
    }

    // // Handle other supported Content-Types (e.g., JSON, plain text, etc.)
    // if (contentType == "application/json" || contentType == "application/x-www-form-urlencoded")
    // {
    //     // Process JSON or form data (e.g., store in database, etc.)
    //     std::cout << "Processing JSON or form data" << std::endl;
    //     sendResponse(200, "OK", "POST processed successfully");
    //     return;
    // }
    // Unsupported Content-Type
    else
        sendErrorResponse(415, "Unsupported Media Type");
}

/**
 * @brief Handles multipart form data uploads from POST requests.
 * 
 * @details Parses the request payload, extracts file content, and saves it to the specified location.
 * 
 * @param [in] path The upload path.
 * @param [in] boundary The boundary string used to separate parts in the multipart data.
 */
// void Client::handleMultipartFormData(const std::string &path, const std::string &boundary)
// {
//     // Derive the upload directory and file path
//     std::string uploadDirectory = _server.getRoot() + "/upload/";
//     std::string fileName = path.substr(path.find_last_of('/') + 1); // Use the path to get the file name

//     if (fileName.empty())
//         fileName = "uploaded_file"; // Fallback to default name
//     std::string filePath = uploadDirectory + fileName;

//     size_t boundaryStart = _requestPayload.find("--" + boundary);
//     if (boundaryStart == std::string::npos)
//     {
//         sendErrorResponse(400, "Invalid multipart body");
//         return;
//     }

//     size_t contentStart = _requestPayload.find("\r\n\r\n", boundaryStart) + 4; // Skip to content
//     size_t contentEnd = _requestPayload.find("\r\n--" + boundary, contentStart);
//     if (contentStart == std::string::npos || contentEnd == std::string::npos)
//     {
//         sendErrorResponse(400, "Malformed multipart section");
//         return;
//     }
//     std::string fileContent = _requestPayload.substr(contentStart, contentEnd - contentStart);
    
//     // Save file content
//     std::ofstream outFile(filePath.c_str(), std::ios::binary);
//     if (!outFile.is_open())
//     {
//         sendErrorResponse(500, "Internal Server Error: Unable to open file for writing");
//         return;
//     }
//     outFile.write(fileContent.c_str(), fileContent.size());
//     outFile.close();
//     sendResponse(200, "OK", "Multipart file uploaded successfully");
// }

void Client::handleMultipartFormData(const std::string &path, const std::string &boundary)
{
    (void)path;

    // Derive the upload directory
    std::string uploadDirectory = _server.getRoot() + "/upload/";
    if (uploadDirectory.back() != '/')
        uploadDirectory += "/";

    std::string boundaryMarker = "--" + boundary;
    std::string closingBoundary = boundaryMarker + "--";

    size_t currentPos = 0;

    // Check if the first boundary is correct
    if (_requestPayload.find(boundaryMarker) != 0)
    {
        sendErrorResponse(400, "Malformed multipart body: Missing or invalid initial boundary");
        return;
    }

    // Iterate over the payload to process all parts
    while (true)
    {
        // Find the next boundary
        size_t boundaryStart = _requestPayload.find(boundaryMarker, currentPos);
        if (boundaryStart == std::string::npos)
        {
            sendErrorResponse(400, "Malformed multipart body: Missing boundary");
            return;
        }

        // Check for closing boundary
        if (_requestPayload.substr(boundaryStart, closingBoundary.size()) == closingBoundary)
            break; // End of parts

        size_t partStart = boundaryStart + boundaryMarker.size() + 2; // Skip boundary and \r\n
        size_t boundaryEnd = _requestPayload.find(boundaryMarker, partStart);

        if (boundaryEnd == std::string::npos)
        {
            sendErrorResponse(400, "Malformed multipart body: Incomplete part or missing boundary");
            return;
        }

        // Extract the part content
        std::string part = _requestPayload.substr(partStart, boundaryEnd - partStart);

        // Extract headers and content
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos)
        {
            sendErrorResponse(400, "Malformed multipart body: Missing headers");
            return;
        }

        std::string headers = part.substr(0, headerEnd);
        std::string content = part.substr(headerEnd + 4); // Skip \r\n\r\n

        // Parse headers for filename
        std::string filename;
        size_t contentDispStart = headers.find("Content-Disposition: ");
        if (contentDispStart != std::string::npos)
        {
            size_t filenameStart = headers.find("filename=\"", contentDispStart);
            if (filenameStart != std::string::npos)
            {
                filenameStart += 10; // Skip 'filename="'
                size_t filenameEnd = headers.find("\"", filenameStart);
                filename = headers.substr(filenameStart, filenameEnd - filenameStart);
            }
        }

        // Default filename if none provided
        if (filename.empty())
            filename = "uploaded_file";

        // Save the file content
        std::string filePath = uploadDirectory + filename;
        std::ofstream outFile(filePath.c_str(), std::ios::binary);
        if (!outFile.is_open())
        {
            sendErrorResponse(500, "Internal Server Error: Unable to open file for writing");
            return;
        }
        outFile.write(content.c_str(), content.size());
        outFile.close();

        // Move to the next boundary
        currentPos = boundaryEnd;
    }

    sendResponse(200, "OK", "Multipart files uploaded successfully");
}


/**
 * @brief Uploads a file to the server.
 * 
 * @details Writes the request payload to a file in the appropriate directory. 
 * The upload directory can be configured per location.
 * 
 * @param [in] path The path where the file will be uploaded.
 */
void Client::uploadFile(const std::string &path)
{
    // Retrieve location configuration
    location_t locationConfig = _server.getLocationConfig(path);
    
    // Debugging to ensure `uploadTo` is used - remove later
    std::cout << "Using uploadTo: " << locationConfig.uploadTo << std::endl;

    // Determine upload directory
    std::string uploadDirectory = _server.getRoot();
    if (!locationConfig.uploadTo.empty())
    {
        if (front(locationConfig.uploadTo) != '/')
            uploadDirectory += "/";
        uploadDirectory += locationConfig.uploadTo;
    }

    // Ensure the upload directory ends with a '/'
    if (back(uploadDirectory) != '/')
        uploadDirectory += "/";

    // Derive the full file path
    std::string fileName = path.substr(path.find_last_of('/') + 1);
    if (fileName.empty())
        fileName = "default_upload.txt"; // Default name
    std::string filePath = uploadDirectory + fileName;

    // Debugging resolved path
    std::cout << "Resolved upload path: " << filePath << std::endl;

    // Open a file to write the payload
    std::ofstream outFile(filePath.c_str(), std::ios::binary);
    if (!outFile.is_open())
    {
        sendErrorResponse(500, "Internal Server Error: Unable to open file for writing");
        return;
    }

    // Write the request payload to the file 
    outFile.write(_requestPayload.c_str(), _requestPayload.size());
    outFile.close();
    sendResponse(200, "OK", "File uploaded successfully");
}

/**
 * @brief Handles a DELETE request to remove resources from the server.
 * 
 * @details This function:
 * - Validates the existence of the target file.
 * - Attempts to delete the file.
 * - Sends appropriate responses based on success or failure.
 * 
 * @param [in] path The path to the file to be deleted.
 */
void Client::handleDeleteRequest(const std::string &path)
{
   
    // Check if the request path starts with "/upload"
    std::string fullPath;
    if (path.find("/upload/") == 0) {
        location_t locationConfig = _server.getLocationConfig(std::string("/upload"));
        
        // Check if the requested method is allowed in the location block
        for (std::vector<std::string>::iterator it = locationConfig.methods.begin(); it != locationConfig.methods.end(); ++it)
        {
            if (*it == "DELETE")
            {
                // The "DELETE" method is allowed, exit the loop.
                break;
            }

            // If we are at the last element and "DELETE" was not found, return an error.
            if (it + 1 == locationConfig.methods.end()) // Use iterator arithmetic since vector supports it.
            {
                sendErrorResponse(405, "Method Not Allowed");
                return;
            }
        }
        fullPath = this->_server.getRoot() + "/upload/" + path.substr(8); // Construct the path for files in the upload directory
    }
    else {
        location_t locationConfig = _server.getLocationConfig(std::string("/"));
        
        // Check if the requested method is allowed in the location block
        for (std::vector<std::string>::iterator it = locationConfig.methods.begin(); it != locationConfig.methods.end(); ++it)
        {
            if (*it == "DELETE")
            {
                // The "DELETE" method is allowed, exit the loop.
                break;
            }

            // If we are at the last element and "DELETE" was not found, return an error.
            if (it + 1 == locationConfig.methods.end()) // Use iterator arithmetic since vector supports it.
            {
                sendErrorResponse(405, "Method Not Allowed");
                return;
            }
        }
        fullPath = this->_server.getRoot() + path; // Construct the path for files in the root directory
    }

    //std::cout << "Attempting to delete: " << fullPath << std::endl;

    // Check if the file exists
    if (!fileExists(fullPath))
    {
		//std::cout << "File does not exist!" << std::endl;
        sendErrorResponse(404, "Not Found");
        return;
    }

    // Attempt to delete the file
    if (std::remove(fullPath.c_str()) != 0)
        sendErrorResponse(403, "Forbidden"); // permission or other errors
    else
        sendResponse(200, "OK", "File Deleted");
}

/**
 * @brief Checks if a file exists at the specified path.
 * 
 * @param [in] path The path to check.
 * @return true if the file exists, false otherwise.
 */
bool Client::fileExists(const std::string& path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
