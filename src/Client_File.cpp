#include "Client.hpp"

// // Handle GET requests
// void Client::handleGetRequest(const std::string& path) { /* ... */ }

// // Handle POST requests
// void Client::handlePostRequest(const std::string &path) { /* ... */ }
// void Client::uploadFile(const std::string &path) { /* ... */ }

// // Handle DELETE requests
// void Client::handleDeleteRequest(const std::string &path) { /* ... */ }


// Handle GET requests
void Client::handleGetRequest(const std::string& path)
{
    // Extract path without query string
    std::string cleanPath = path.substr(0, path.find('?'));

    // Construct the full file path
    std::string filePath = this->_server.getRoot() + cleanPath;

    // If the path is "/", serve the default index file
    if (cleanPath == "/")
        filePath = this->_server.getRoot() + "/index.html";

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
    {
        // Send a 404 error if the file is not found
        sendErrorResponse(404, "Not Found");
    }
}

// Handle POST requests
//TODO Add a check for allowed file extensions(?)
//TODO parse multipart/form-data requests

void Client::handlePostRequest(const std::string &path)
 {
    // Retrieve location configuration for this request
    location_t locationConfig = _server.getLocationConfig(path);
    std::cout << "Path: " << path << ", Has CGI: " << locationConfig.hasCGI 
          << ", CGI Extension: " << locationConfig.cgiExtension << std::endl;
    
    // Check if the path is a CGI script
    if (locationConfig.hasCGI && path.find(locationConfig.cgiExtension) != std::string::npos)
    {
        std::cout << "Executing CGI script: " << path << std::endl;
        executeCgi(locationConfig.cgiPath, path); // Placeholder for actual CGI execution
        return;
    }
    else
        std::cout << "Not a CGI script" << std::endl;

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
            contentType == "pdf") // we can add more supported content types
    {
        uploadFile(path);
        std::cout << "Handling Content-Type: " << contentType << " for path: " << path << std::endl;
    }
    // Unsupported Content-Type
    else
        sendErrorResponse(415, "Unsupported Media Type");
}

// Handle multipart form data
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

// Upload a file to the server
void Client::uploadFile(const std::string &path)
{
    // Retrieve location configuration
    location_t locationConfig = _server.getLocationConfig(path);
    
    // Debugging to ensure `uploadTo` is used
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

    // Send a success response
    sendResponse(200, "OK", "File uploaded successfully");
}

// Handle DELETE requests
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

bool Client::fileExists(const std::string& path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
