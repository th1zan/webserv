#include "Client.hpp"

// void Client::handleClientRequest()
// bool Client::_checkRequest()
// bool Client::_checkFirstLine(std::stringstream &ss)
// bool Client::_checkAndGetHeaders(std::stringstream &ss)
// bool Client::_checkAndGetPayload(std::stringstream &ss)
// bool Client::isUrlValid(const std::string &url) const 


/**
 * @brief Handles the client's request by validating, routing, and processing it.
 * 
 * This function performs the following steps:
 * - Validates the client's HTTP request.
 * - Determines the resource path and root directory.
 * - Matches the request with server locations or handles redirects.
 * - Routes the request to the appropriate handler based on the HTTP method.
 * 
 * Supported methods:
 * - GET
 * - POST
 * - DELETE
 */
void Client::handleClientRequest()
{
    // Validate the request format
    if (!_checkRequest())
    {
        sendErrorResponse(400, "Bad Request: Invalid request format");
        return;
    }

    // Prepare root and resource paths
    std::string root = _server.getRoot();
    std::string resource = _resourcePath;

    // Validate the requested location or handle a redirect
	int locationStatus = _checkLocation(root, resource, 0);
    if (locationStatus == -1) // Location not found
    {
        sendErrorResponse(404, "Not Found: Invalid resource location");
        return;
    }
    else if (locationStatus == 1) // Redirect handled
        return;

    // Route the request based on the HTTP method
    if (_method == "GET")
        handleGetRequest(resource);
    else if (_method == "POST")
        handlePostRequest(resource);
    else if (_method == "DELETE")
        handleDeleteRequest(resource);
    else
        sendErrorResponse(501, "Not Implemented");
}

/**
 * @brief Validates the client's HTTP request format and content.
 * 
 * @details Checks the following:
 * - Request line (method, path, and version).
 * - Headers (e.g., Host, Content-Type).
 * - Payload for POST requests.
 * 
 * @return true if the request is valid, false otherwise.
 */
bool Client::_checkRequest()
{
    std::stringstream ss(_request);
    _headers.clear();
    _requestPayload.clear();

    if (!_checkFirstLine(ss) || !_checkAndGetHeaders(ss) || !_checkAndGetPayload(ss))
        return false;
	// TODO - add chunked transfer encoding?
    return true;
}

/**
 * @brief Validates the first line of the HTTP request.
 * 
 * @details Extracts the method, resource path, and HTTP version from the request line.
 * Validates the following:
 * - Supported HTTP versions (only HTTP/1.1 is allowed).
 * - Supported methods (GET, POST, DELETE).
 * - Valid URL path characters.
 * 
 * @param [in] ss String stream containing the HTTP request.
 * @return true if the first line is valid, false otherwise.
 */
bool Client::_checkFirstLine(std::stringstream &ss)
{
    std::string line;
    std::getline(ss, line);

    std::istringstream lineStream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (lineStream >> token)
        tokens.push_back(token);
    if (tokens.size() != 3)
    {
        sendErrorResponse(400, "Bad Request: Invalid Request Line");
        return false;
    }
    _method = tokens[0];
    _resourcePath = tokens[1];
    std::string version = tokens[2];
	// Log for debugging
    // std::cout << "Parsed method: " << _method
    //           << ", resource: " << _resourcePath
    //           << ", version: " << version << std::endl;

    // Validate the HTTP version
    if (version != "HTTP/1.1")
    {
        if (version == "HTTP/1.0") // Specific check for HTTP/1.0
            sendErrorResponse(505, "HTTP Version Not Supported: Use HTTP/1.1");
        else
            sendErrorResponse(400, "Bad Request: Invalid HTTP Version");
        return false;
    }
    // Validate the method
    if (_method != "GET" && _method != "POST" && _method != "DELETE")
    {
        sendErrorResponse(501, "Not Implemented");
        return false;
    }
    // Validate the url path characters
    if (!isUrlValid(_resourcePath))
    {
        sendErrorResponse(400, "Bad Request: Invalid Character in URL");
        return false;
    }
    return true;
}

/**
 * @brief Checks if a given URL is valid.
 * 
 * @details A valid URL contains only the following characters:
 * - Alphanumeric characters
 * - `-._~:/?#[]@!$&'()*+,;=%`
 * 
 * @param [in] url The URL to validate.
 * @return true if the URL is valid, false otherwise.
 */
bool Client::isUrlValid(const std::string &url) const
{
    const std::string allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "0123456789-._~:/?#[]@!$&'()*+,;=%";
    for (std::string::const_iterator it = url.begin(); it != url.end(); ++it)
    {
        if (allowedChars.find(*it) == std::string::npos)
            return false;
    }
    return true;
}

//TODO fix the bug when this command passes curl -v --http1.1 --header "" http://localhost:8080/ Host header should not be empty
/**
 * @brief Parses and validates the headers from the HTTP request.
 * 
 * @details Validates the following:
 * - Presence of mandatory headers like Host.
 * - Content-Type for POST requests.
 * - Malformed headers.
 * 
 * @param [in] ss String stream containing the HTTP request.
 * @return true if the headers are valid, false otherwise.
 */
bool Client::_checkAndGetHeaders(std::stringstream &ss)
{
    std::string line;
    while (std::getline(ss, line))
    {
        if (line == "\r" || line.empty())
            break;

        size_t colon = line.find(':');
        if (colon == std::string::npos)
        {
            sendErrorResponse(400, "Bad Request: Malformed Header");
            return false;
        }
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        _headers[key] = value;
    }
    if (_headers.find("Host") == _headers.end() || _headers["Host"].empty())
    {
        sendErrorResponse(400, "Bad Request: Missing Host Header");
        return false;
    }
    // //fix the bug when this command passes curl -v --http1.1 --header "" http://localhost:8080/ Host header should not be empty
    // // the code below didnt fix it
    // if (_headers["Host"].empty())
    // {
    //     sendErrorResponse(400, "Bad Request: Empty Host Header");
    //     return false;
    // }

    // Validate Content-Type (for POST requests - optional check, can be removed later)
    if (_method == "POST")
    {
        if (_headers.find("Content-Type") == _headers.end())
        {
            sendErrorResponse(400, "Bad Request: Missing Content-Type Header");
            return false;
        }
    }
    return true;
}

/**
 * @brief Parses and validates the payload for POST requests.
 * 
 * @details Validates the following:
 * - Content-Length header is present and valid.
 * - Payload size does not exceed the server's maximum allowed size.
 * - Payload is completely received.
 * 
 * @param [in] ss String stream containing the HTTP request.
 * @return true if the payload is valid, false otherwise.
 */
bool Client::_checkAndGetPayload(std::stringstream &ss)
{
    // Only applicable for POST requests
    if (this->_method != "POST")
        return true;

    // Check if Content-Length header is present
    std::map<std::string, std::string>::iterator it = this->_headers.find("Content-Length");
    if (it == this->_headers.end())
    {
        sendErrorResponse(411, "Length Required");
        return false;
    }

    // Validate that Content-Length is a positive integer
    long contentLength = -1;
    try
    {
        contentLength = std::stol(it->second);
        if (contentLength < 0)
        {
            sendErrorResponse(400, "Bad Request: Invalid Content-Length");
            return false;
        }
    }
    catch (const std::exception &)
    {
        sendErrorResponse(400, "Bad Request: Invalid Content-Length");
        return false;
    }

    // Check against max payload size
    long maxPayloadSize = this->_server.getClientMaxBodySize();
    if (contentLength > maxPayloadSize)
    {
        sendErrorResponse(413, "Payload Too Large");
        return false;
    }

    // Resize the payload container and read the content
    this->_requestPayload.resize(contentLength);
    ss.read(&this->_requestPayload[0], contentLength);

    // Validate if the entire payload was read
    if (ss.gcount() != contentLength)
    {
        sendErrorResponse(400, "Bad Request: Incomplete Payload");
        return false;
    }
    return true; // Payload is valid
}

/**
 * @brief Matches the requested resource with server locations and updates root path if applicable.
 * 
 * @details Performs the following operations:
 * - Finds the best-matching location block based on the resource path.
 * - Handles redirects if specified in the matched location.
 * - Checks if the method is allowed in the location.
 * - Updates the root directory if specified in the location.
 * 
 * @param [in, out] root Reference to the root directory to be updated.
 * @param [in, out] resource Reference to the resource path.
 * @param [in] loopCount Counter to prevent infinite loops in redirects.
 * @return -1 if no matching location is found, 1 if a redirect is handled, 0 otherwise.
 */
int Client::_checkLocation(std::string &root, std::string &resource, size_t loopCount)
{
    if (loopCount > MAX_LOOP_COUNT)
        return -1; // Indicates a loop error

    const std::map<std::string, location_t> &locations = _server.getLocations();
    const location_t *matchedLocation = NULL;
    size_t matchedPrefixLength = 0;

    // Find the best matching location
    for (std::map<std::string, location_t>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if (resource.find(it->first) == 0 && it->first.length() > matchedPrefixLength)
        {
            matchedLocation = &(it->second);
            matchedPrefixLength = it->first.length();
        }
    }
    if (matchedLocation)
    {
        // Handle redirects
        if (!matchedLocation->redirect.empty())
        {
            sendRedirectResponse(302, matchedLocation->redirect); // Redirect
            return 1; // Redirect handled
        }
        // Check if the method is allowed
        if (std::find(matchedLocation->methods.begin(), matchedLocation->methods.end(), _method) == matchedLocation->methods.end())
            throw std::runtime_error("405 Method Not Allowed");

        // Update the root if specified
        if (!matchedLocation->root.empty())
            root = matchedLocation->root;

        return 0; // Successfully matched a location
    }
    return -1; // No matching location found
}
