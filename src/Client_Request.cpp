/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client_Request.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/29 16:09:28 by zsoltani          #+#    #+#             */
/*   Updated: 2024/12/03 16:38:28 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>

// void Client::handleClientRequest()
// bool Client::_checkRequest()
// bool Client::_checkFirstLine(std::stringstream &ss)
// bool Client::_checkAndGetHeaders(std::stringstream &ss)
// bool Client::_checkAndGetPayload(std::stringstream &ss)
// bool Client::isUrlValid(const std::string &url) const 
// int Client::_checkLocation(std::string &root, std::string &resource, size_t loopCount)

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
    // Set response as "sent" and record the request time
    this->_sentRequest = true;
    this->_lastRequest = std::time(NULL);
    
    // Log the start of request handling
    //std::cout << "[DEBUG] Start handling request for client on socket: " << _socket << std::endl;
    // Validate the request format
    if (!_checkRequest())
    {
        sendErrorResponse(400, "Bad Request: Invalid request format");
        std::cerr << "[ERROR] Invalid request format received." << std::endl;
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
        std::cerr << "[ERROR] Resource location not found: " << resource << std::endl;
        return;
    }
    else if (locationStatus == 1) // Redirect handled
    {
        std::cout << "[DEBUG] Redirect handled for resource: " << resource << std::endl;
        return;
    }
    // Route the request based on the HTTP method
    if (_method == "GET")
        handleGetRequest(resource);
    else if (_method == "POST")
        handlePostRequest(resource);
    else if (_method == "DELETE")
        handleDeleteRequest(resource);
    else
        sendErrorResponse(501, "Not Implemented");
        
    // Clear request and payload after processing
    this->_request.clear();
    this->_requestPayload.clear();
    this->_sentRequest = false;

    // Log the successful processing of the request
    // std::cout << "[DEBUG] Successfully handled request for client on socket: " << _socket << std::endl;
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
    std::cerr << "DEBUG: Raw request:\n" << _request << std::endl; // Debug remove later
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
    std::string decodedUrl = decodeUrl(url); // Decode the URL first
    for (std::string::const_iterator it = decodedUrl.begin(); it != decodedUrl.end(); ++it)
    {
        if (allowedChars.find(*it) == std::string::npos)
            return false; // Invalid character found
    }
    return true; // All characters are valid
}

/**
 * @brief Decodes percent-encoded characters in a URL.
 * 
 * @details Converts URL-encoded sequences (e.g., "%20") back to their 
 * original characters (e.g., space). This is necessary because browsers 
 * automatically encode invalid characters in URLs, and the server needs 
 * to decode them to validate and process the URL correctly.
 * 
 * @param [in] url The URL to decode.
 * @return A decoded URL with all percent-encoded characters replaced by their original values.
 */
std::string Client::decodeUrl(const std::string &url) const
{
    std::string decoded;
    for (size_t i = 0; i < url.length(); ++i)
    {
        if (url[i] == '%' && i + 2 < url.length())
        {
            // Convert percent-encoded sequence to character
            char hex[3] = {url[i + 1], url[i + 2], '\0'};
            decoded += static_cast<char>(strtol(hex, NULL, 16));
            i += 2; // Skip the next two characters
        }
        else
        {
            decoded += url[i];
        }
    }
    return decoded;
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
    bool hostHeaderFound = false;

    while (std::getline(ss, line))
    {
        stringTrim(line); // Trim whitespace around the line
        if (line == "\r" || line.empty())
            break; // End of headers

        // //check if line starts with "Host:" 
        // if (startsWith(line, "Host:"))
        //     hostHeaderFound = true;

        // Find the colon for key-value separation
        size_t colon = line.find(':');
        if (colon == std::string::npos)
        {
            sendErrorResponse(400, "Bad Request: Malformed Header");
            return false;
        }
        // Extract key and value
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        stringTrim(key);
        stringTrim(value);
        // Validate header key and value
        if (key.empty() || value.empty())
        {
            sendErrorResponse(400, "Bad Request: Empty Header Key or Value");
            return false;
        }
        // Store the header in the map
        _headers[key] = value;
        if (key == "Host")
        {
            hostHeaderFound = true;
            // Validate the Host header
            if (value.empty())
            {
                sendErrorResponse(400, "Bad Request: Empty Host Header");
                return false;
            }
        }
    }
    // Ensure Host header is present
    if (!hostHeaderFound)
    {
        std::cerr << "DEBUG: Host header is missing!" << std::endl; // Debug remove later
        sendErrorResponse(400, "Bad Request: Missing Host Header");
        return false;
    }
    //Validate Content-Type (for POST requests - optional check, can be removed later)
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
    std::cout << "DEBUG: Populated _requestPayload: " << _requestPayload << std::endl;
    // Validate if the entire payload was read
    if (ss.gcount() != contentLength)
    {
        sendErrorResponse(400, "Bad Request: Incomplete Payload");
        return false;
    }
    return true; // Payload is valid
}

/**
 * @brief Matches a resource path to a server's location block.
 * 
 * Searches for the best matching location block based on a longest-prefix match.
 * Handles redirects, validates HTTP methods, and updates the root directory if applicable.
 * 
 * @param root [in, out] Reference to the root directory path, updated if a new root is specified.
 * @param resource [in, out] Reference to the requested resource path, may be modified for redirects.
 * @param loopCount [in] Counter to prevent infinite redirects.
 * 
 * @return 
 * - `0` if a location block is matched.
 * - `1` if a redirect is handled.
 * - `-1` if no location matches or an error occurs.
 * 
 * @details Sends error responses for invalid methods or unmatched locations. 
 * Validates allowed methods and handles redirections where applicable.
 */
int Client::_checkLocation(std::string &root, std::string &resource, size_t loopCount)
{
    if (loopCount > MAX_LOOP_COUNT)
        return -1;

    const std::map<std::string, location_t> &locations = _server.getLocations();
    std::map<std::string, location_t>::const_iterator matchedLocation = locations.end();
    size_t matchedPrefixLength = 0;

    // Find the best matching location
    for (std::map<std::string, location_t>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if (resource.find(it->first) == 0 && it->first.size() > matchedPrefixLength)
        {
            matchedLocation = it;
            matchedPrefixLength = it->first.size();
        }
    }

    if (matchedLocation == locations.end())
        return -1; // No matching location found

    const location_t &loc = matchedLocation->second;

    // Handle redirects
    if (!loc.redirect_path.empty())
    {
        sendRedirectResponse(ft_stoll(loc.redirect_err), loc.redirect_path);
        return 1; // Redirect handled
    }

    // Check if the method is allowed
    if (std::find(loc.methods.begin(), loc.methods.end(), _method) == loc.methods.end())
    {
        // Generate a list of allowed methods
        std::string allowedMethods;
        for (std::vector<std::string>::const_iterator it = loc.methods.begin(); it != loc.methods.end(); ++it)
        {
            if (!allowedMethods.empty())
                allowedMethods += ", ";
            allowedMethods += *it;
        }

        // Send error response with allowed methods
        std::string errorMessage = "405 Method Not Allowed. Allowed methods: " + allowedMethods;
        sendErrorResponse(405, errorMessage);
        std::cerr << "[ERROR] Method not allowed. Allowed methods: " << allowedMethods << std::endl;
        return -1; // Return an error code
    }

    // Update the root if specified
    if (!loc.root.empty())
        root = loc.root;

    return 0; // Successfully matched a location
}
