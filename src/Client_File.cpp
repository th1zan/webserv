#include "Client.hpp"

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
    std::string cleanPath = path.substr(0, path.find('?'));

    // Construct the full file path
    std::string filePath = this->_server.getRoot() + cleanPath;

    // If the path is "/", serve the default index file
    if (cleanPath == "/")
        filePath = this->_server.getRoot() + "/index.html";
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
        if (requestPath.back() != '/') autoindex << '/';
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
    // Retrieve location configuration for this request
    location_t locationConfig = _server.getLocationConfig(path);
    std::cout << "Path: " << path << ", Has CGI: " << locationConfig.hasCGI 
          << ", CGI Extension: " << locationConfig.cgiExtension << std::endl; // Debugging
    
    // Check if the path is a CGI script
    if (isCgiPath(path, locationConfig))
    {
        std::cout << "Executing CGI script: " << path << std::endl;
        executeCgi(locationConfig.cgiPath, path); // Placeholder for actual CGI execution
        return;
    }
    else
        std::cout << "Not a CGI script" << std::endl; // Debugging - remove later

    // Validate Content-Type header
    std::map<std::string, std::string>::iterator it = _headers.find("Content-Type");
    std::string contentType;
    if (it == _headers.end()) // Fallback to text/plain if Content-Type is missing
        contentType = "text/plain";
    else
    {
        contentType = it->second;
        stringTrim(contentType); // Trim whitespace
    }
    // degug to remove
    std::cout << "Normalized Content-Type: " << contentType << std::endl;
    // Handle multipart form data
    if (contentType.find("multipart/form-data") != std::string::npos)
    {
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos)
        {
            sendErrorResponse(400, "Missing boundary parameter in Content-Type");
            return;
        }
        std::string boundary = contentType.substr(boundaryPos + 9); // Skip "boundary="
        handleMultipartFormData(path, boundary);
    } 
    // Handle plain text, JSON or x-www-form-urlencoded payloads (if we handle them)
    else if (contentType == "text/plain" || contentType == "application/json" || contentType == "application/x-www-form-urlencoded" || 
            contentType == "image/png" || contentType == "image/jpeg" || contentType == "image/gif" ||
            contentType == "application/pdf" || contentType == "application/xml" || contentType == "text/html") // we can add more supported content types
    {
        uploadFile(path);
        std::cout << "Handling Content-Type: " << contentType << " for path: " << path << std::endl; // Debugging - remove later
    }
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
void Client::handleMultipartFormData(const std::string &path, const std::string &boundary)
{
    // Derive the upload directory and file path
    std::string uploadDirectory = _server.getRoot() + "/upload/";
    std::string fileName = path.substr(path.find_last_of('/') + 1); // Use the path to get the file name

    if (fileName.empty())
        fileName = "uploaded_file"; // Fallback to default name
    std::string filePath = uploadDirectory + fileName;

    size_t boundaryStart = _requestPayload.find("--" + boundary);
    if (boundaryStart == std::string::npos)
    {
        sendErrorResponse(400, "Invalid multipart body");
        return;
    }

    size_t contentStart = _requestPayload.find("\r\n\r\n", boundaryStart) + 4; // Skip to content
    size_t contentEnd = _requestPayload.find("\r\n--" + boundary, contentStart);
    if (contentStart == std::string::npos || contentEnd == std::string::npos)
    {
        sendErrorResponse(400, "Malformed multipart section");
        return;
    }
    std::string fileContent = _requestPayload.substr(contentStart, contentEnd - contentStart);
    
    // Save file content
    std::ofstream outFile(filePath.c_str(), std::ios::binary);
    if (!outFile.is_open())
    {
        sendErrorResponse(500, "Internal Server Error: Unable to open file for writing");
        return;
    }
    outFile.write(fileContent.c_str(), fileContent.size());
    outFile.close();
    sendResponse(200, "OK", "Multipart file uploaded successfully");
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
        if (locationConfig.uploadTo.front() != '/')
            uploadDirectory += "/";
        uploadDirectory += locationConfig.uploadTo;
    }

    // Ensure the upload directory ends with a '/'
    if (uploadDirectory.back() != '/')
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
    if (path.find("/upload") == 0)
        fullPath = this->_server.getRoot() + "/upload" + path.substr(7); // Construct the path for files in the upload directory
    else
        fullPath = this->_server.getRoot() + path; // Construct the path for files in the root directory

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
