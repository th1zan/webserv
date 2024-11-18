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
void Client::handlePostRequest(const std::string &path)
{
    std::string uploadDirectory = _server.getRoot() + "/upload/";
    std::string fileName = path.substr(path.find_last_of('/') + 1);

    // Check if a filename is provided in the path
    if (fileName.empty())
    {
        sendErrorResponse(400, "Bad Request: No file name provided");
        return;
    }

    // Construct the full file path
    std::string filePath = uploadDirectory + fileName;

    // Try uploading the file
    std::ofstream outFile(filePath.c_str(), std::ios::binary);
    if (!outFile.is_open())
    {
        sendErrorResponse(500, "Internal Server Error: Unable to open file for writing");
        return;
    }

    // Write the payload to the file
    outFile.write(_requestPayload.c_str(), _requestPayload.size());
    outFile.close();

    // Send a success response
    sendResponse(200, "OK", "File uploaded successfully");
}

void Client::uploadFile(const std::string &path)
{
    // Derive the file path directly in the upload directory
    std::string filePath = _server.getRoot() + path;

    // Open a file to write the payload
    std::ofstream outFile(filePath.c_str(), std::ios::binary);
    if (!outFile.is_open())
    {
        sendErrorResponse(500, "Internal Server Error");
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

bool fileExists(const std::string& path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
