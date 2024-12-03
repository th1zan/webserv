/**
 * @file Parser_3_checkLocation.cpp
 * @brief This file containes the functions used to check the Location
 * parameters in each 'Server'.
 */

#include "Parser.hpp"
#include <sstream>
#include <string>

/**
 * The function _checkLocationParam() in C++ calls two other functions to check
 * directory name and value.
 */
void Parser::_checkLocationParam() {
  this->_checkLocDirName();
  this->_checkLocDirValue();
}

/**
 * The function `_checkLocDirName` iterates through a vector of maps, checking
 * for forbidden parameters in each map. The parameters come from Location blocs
 * in the cnfig file.
 */
void Parser::_checkLocDirName() {

  for (std::vector<std::map<std::string, std::string> >::const_iterator itVec =
           this->_tempLocationMapVector.begin();
       itVec != this->_tempLocationMapVector.end(); itVec++) {
    _tempLocationConfigMap = *itVec;

    // No need to control LOCATION parameter and any mandatory parameters for
    // location
    //  std::string const mandatoryParam[] = {LOCATION};

    // forbiden = Location parameter
    std::string const forbiddenParam[] = {LISTEN,   HOST,     ROOT_LOC, INDEX,
                                          MAX_SIZE, SERVER_N, ERROR_P};

    for (int i = 0; i < 7; i++) {
      if (this->_tempLocationConfigMap.find(forbiddenParam[i]) !=
          this->_tempLocationConfigMap.end())
        throw std::runtime_error(
            ERR_LOC_FORBIDDEN_DIRECTIVE(forbiddenParam[i]));
    }
  }
}

/**
 * The function `_checkLocDirValue` iterates through a vector of maps and calls
 * different check functions based on the key in each map. It checks the value
 * of each directive in a Location bloc.
 */
void Parser::_checkLocDirValue() {
  for (std::vector<std::map<std::string, std::string> >::iterator itVec =
           this->_tempLocationMapVector.begin();
       itVec != this->_tempLocationMapVector.end(); ++itVec) {
    std::map<std::string, std::string> &locMap = *itVec;

    for (std::map<std::string, std::string>::iterator itMap = locMap.begin();
         itMap != locMap.end(); ++itMap) {
      if (itMap->first == LOCATION) {
        this->_checkLocation(itMap->second);
      } else if (itMap->first == ALLOW_M) {
        this->_checkAllowM(itMap->second);
      } else if (itMap->first == TRY) {
        this->_checkTry(itMap->second);
      } else if (itMap->first == RETURN) {
        this->_checkReturn(itMap->second);
      } else if (itMap->first == AUTOID) {
        this->_checkAutoID(itMap->second);
      } else if (itMap->first == ROOT_LOC) {
        this->_checkRootLoc(itMap->second);
      } else if (itMap->first == UPLOAD) {
        this->_checkUpload(itMap->second);
      } else if (itMap->first == CGI_P) {
        this->_checkCgiP(itMap->second, locMap); // Pass locMap by reference
      } else if (itMap->first == CGI_E) {
        this->_checkCgiE(itMap->second);
      }
    }
  }
}

/**
 * The `_checkPath` function verifies the existence, type, and write permission
 * of a specified path in C++.
 *
 * @param path The `path` parameter is a string that represents the path to a
 * file or directory that needs to be checked for existence, type (directory or
 * file), and write permission.
 * @param isDir The `isDir` parameter in the `_checkPath` function is a boolean
 * flag that indicates whether the path being checked should be a directory
 * (`true`) or not (`false`). This parameter helps the function determine
 * whether the path should be validated as a directory or a regular file.
 * @param hasWPerm The `hasWPerm` parameter in the `_checkPath` function is a
 * boolean flag that indicates whether the path should have write permission
 * checked. If `hasWPerm` is set to `true`, the function will verify if the path
 * has write permission by checking the access rights using the `access
 */
void Parser::_checkPath(std::string &path, bool isDir, bool hasWPerm) {
  struct stat info;

  std::string fullPath = this->_tempRootDirPath + path;

  // Check for existence
  if (access(fullPath.c_str(), F_OK) != 0) {
    throw std::runtime_error(ERR_DIRECTORY(fullPath));
  }

  // Get file information
  if (stat(fullPath.c_str(), &info) != 0) {
    throw std::runtime_error(ERR_FILE_INFO(fullPath));
  }

  // Check if it's a directory or a file
  if ((info.st_mode & S_IFDIR) != 0) { // It's a directory
    if (!isDir) {
      throw std::runtime_error(ERR_DIRECTORY(fullPath));
    }
  } else if ((info.st_mode & S_IFREG) != 0) { // It's a regular file
    if (isDir) {
      throw std::runtime_error(ERR_FILE(fullPath));
    }
  } else {
    throw std::runtime_error(ERR_NOT_FILE_NOT_DIR(fullPath));
  }

  // Check for write access if hasWPerm is true
  if (hasWPerm && access(fullPath.c_str(), W_OK) != 0) {
    throw std::runtime_error(ERR_PATH(fullPath));
  }
}

/**
 * The function `_checkLocation` removes trailing curly braces from a string
 * representing a location value.
 *
 */
void Parser::_checkLocation(std::string &dirValue) {

  /*
  The value after `location` determines which segment of a URL or which type of
  file the block applies to. It can be a specific path, a file extension, or a
  regex pattern.
  */

  // ANY verification for now, only delete the ending '{'
  while (!dirValue.empty() && (std::isspace(dirValue[dirValue.size() - 1]) ||
                               dirValue[dirValue.size() - 1] == '{')) {
    dirValue.erase(dirValue.size() - 1);
  }

  // if (dirValue.empty() || dirValue[0] != '/') {
  // 	throw std::runtime_error(ERR_LOCATION(dirValue));
  // }
}

/**
 * The function `_checkAllowM` validates if the methods in a given string are
 * allowed (GET, POST, DELETE).
 *
 */
void Parser::_checkAllowM(std::string &dirValue) {
  // delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);

  // Set of allowed methods
  std::set<std::string> allowedMethods;
  allowedMethods.insert("GET");
  allowedMethods.insert("POST");
  allowedMethods.insert("DELETE");

  // Split the methods by space and check validity
  std::stringstream ss(dirValue);
  std::string method;
  while (ss >> method) {
    if (allowedMethods.find(method) == allowedMethods.end()) {
      throw std::runtime_error(ERR_INVALID_METHOD(method, dirValue));
    }
  }
}

/**
 * The function `_checkTry` checks if a file exists at a specified directory
 * path and throws a runtime error if the file does not exist.
 *
 */
void Parser::_checkTry(std::string &dirValue) {

  /*function seems identical to _checkIndex !!! probably possible to use it*/

  // delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);

  // make fullPath to the file
  std::string fullPath = this->_tempRootDirPath + "/" + dirValue;

  // Vérifier si le fichier existe
  int fd = open(fullPath.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::runtime_error(ERR_FILE(fullPath));
  }
  // Fermer le descripteur de fichier si l'ouverture a réussi
  close(fd);
}

/**
 * The function `_checkReturn` checks and cleans a given string representing a
 * directory path.
 *
 * @param dirValue The `dirValue` parameter in the `_checkReturn` function is a
 * reference to a `std::string` variable. It is used to store a directory value
 * that will be checked and processed within the function.
 */
void Parser::_checkReturn(std::string &dirValue) {

  /*function seems identical to _checkRoot !!! probably possible to use it*/

  // Delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);

  std::istringstream iss(dirValue);
  // the _checkPath is done later
  std::vector<std::string> tokens;
  std::string word;
  std::string redirect_path;
  std::string redirect_err;

  while (iss >> word) {
    tokens.push_back(word);
  }

  if (tokens.size() == 2) {
    redirect_err = tokens[0];
    redirect_path = tokens[1];
    if (redirect_err != "301" && redirect_err != "302") {
      throw std::runtime_error("Error in 'return' directive: the status code needs to be 301 or 302. ");
  } else if (tokens.size() == 1) {
    redirect_path = tokens[0];
  } else if (tokens.size() < 2){
      throw std::runtime_error("Error in 'return' directive: more than 2 arguments");
  }
  this->_checkPath(redirect_path, false);
  }
}

/**
 * The function `_checkAutoID` checks if the input string is either "on" or
 * "off" after converting it to lowercase.
 */
void Parser::_checkAutoID(std::string &dirValue) {
  // Delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);

  for (std::string::iterator it = dirValue.begin(); it != dirValue.end(); it++)
    *it = ::tolower(*it);

  if (dirValue != "on" && dirValue != "off")
    throw std::runtime_error(ERR_AUTOINDEX);
}

/**
 * The function `_checkRootLoc` checks and cleans a directory path.
 *
 */
void Parser::_checkRootLoc(std::string &dirValue) {
  // Delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);
  this->_checkPath(dirValue, true);
}

/**
 * The function `_checkUpload` checks and cleans a directory path.
 *
 */
void Parser::_checkUpload(std::string &dirValue) {
  // Delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);
  this->_checkPath(dirValue, true, true);
}

/**
 * @brief Checks for executable CGI files in a specified directory path.
 *
 * This function inspects a directory to determine if it contains any
 * executable files that can be used as CGI scripts. It performs the
 * following operations:
 * - Cleans up the directory path by removing any trailing semicolon.
 * - Validates the directory path to ensure it is accessible and correctly
 * formatted.
 * - Opens the directory and iterates through its entries, checking each file.
 * - Identifies regular files that have execute permissions, indicating
 * potential CGI scripts.
 * - Updates a temporary configuration map to reflect the presence of CGI files.
 *
 * @param dirValue A reference to the string representing the directory path to
 * be checked. This path is cleaned and validated within the function.
 *
 * @throws std::runtime_error If the directory cannot be opened, an exception is
 * thrown with an appropriate error message.
 *
 * @note The function modifies the `_tempLocationConfigMap` to set "hasCgi" to
 * "true" if any executable CGI files are found in the directory.
 */

void Parser::_checkCgiP(std::string &dirValue,
                        std::map<std::string, std::string> &locMap) {
  // Delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);

  // check directory
  this->_checkPath(dirValue, true);

  std::string fullPath = this->_tempRootDirPath + dirValue;
  //  open directory
  DIR *dir = opendir(fullPath.c_str());
  if (!dir) {
    throw std::runtime_error(ERR_OPEN_DIR(dirValue));
  }

  struct dirent *entry;
  // loop to list the files in the directory and check CGI
  while ((entry = readdir(dir)) != NULL) {
    // ignore the special files '.' and '..'
    if (entry->d_name[0] == '.') {
      continue;
    }

    // get the filePath
    std::string filePath = fullPath + "/" + entry->d_name;
    struct stat fileInfo;

    // Get info
    if (stat(filePath.c_str(), &fileInfo) == 0) {
      // check if the file is regular (not a directory and not a link/alias) AND
      // executable
      if ((fileInfo.st_mode & S_IFREG) && (fileInfo.st_mode & S_IXUSR)) {
        locMap[HAS_CGI] =
            "true"; // Modification directement dans locationConfigMap
        break;
      }
    } else {
      std::cerr << "CGI: Error checking file: " << filePath << std::endl;
    }
  }
  closedir(dir);
}

/**
 * The function `_checkCgiE` checks if a given file extension is supported for
 * CGI processing.
 *
 * @param dirValue The `dirValue` parameter in the `_checkCgiE` function is a
 * reference to a `std::string` variable. It is used to store the directory
 * value that needs to be checked for a valid CGI extension {".cgi", ".pl",
 * ".py", ".sh", ".php"}.
 */
void Parser::_checkCgiE(std::string &dirValue) {
  // Delete ending ';' if necessary to get a cleaner string later
  this->_delEndSemiColon(dirValue);

  // list of supported extension (created in a dedicated function
  // 'getSupportedExtensions')
  //  const std::set<std::string> supportedExtensions = {".cgi", ".pl", ".py",
  //  ".sh", ".php"}; //initilisation list not possible in CPP98
  const std::set<std::string> &supportedExtensions = getSupportedExtensions();

  // check if there is a '.'
  if (dirValue.empty() || dirValue[0] != '.') {
    throw std::runtime_error("Extension must start with a point (.)");
  }

  // //check if the extension is supported (in the list)
  if (supportedExtensions.find(dirValue) == supportedExtensions.end()) {
    throw std::runtime_error("Unsupported CGI extension: " + dirValue);
  }
}

std::set<std::string> Parser::getSupportedExtensions() {
  std::set<std::string> extensions;
  extensions.insert(".cgi");
  extensions.insert(".pl");
  extensions.insert(".py");
  extensions.insert(".sh");
  extensions.insert(".php");
  return extensions;
}

/*******************************************/
// Following functions are probably not needed
//- isValidUrl
//- urlExists
/*******************************************/

/*
bool isValidUrl(const std::string& url) {
        // Check if the URL starts with "http://" or "https://"
        if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://") {
                return false;
        }

        size_t pos = url.find("://");
        if (pos == std::string::npos) {
                return false; // "://" not found
        }

        // Check for a valid domain (simplified, allows letters, digits, dots,
and hyphens) size_t start = pos + 3; // Move past "://" size_t end =
url.find('/', start); if (end == std::string::npos) { end = url.length(); // No
path found, use end of string
        }

        std::string domain = url.substr(start, end - start);
        for (size_t i = 0; i < domain.length(); ++i) {
                char c = domain[i];
                if (!std::isalnum(c) && c != '.' && c != '-') {
                        return false; // Invalid character in domain
                }
        }

        // Check if an optional port is present
        size_t port_pos = domain.find(':');
        if (port_pos != std::string::npos) {
                std::string port = domain.substr(port_pos + 1);
                for (size_t i = 0; i < port.length(); ++i) {
                        if (!std::isdigit(port[i])) {
                                return false; // Port contains non-digit
characters
                        }
                }
        }

        return true; // URL is valid
}

#include <curl/curl.h>
bool urlExists(const std::string& url) {
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // don't need the body of
the response curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Timeout 10 sec

        // delete request
        CURLcode res = curl_easy_perform(curl);

        // check the answer
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        curl_easy_cleanup(curl);

        // check if answer is 200 (OK)
        return (res == CURLE_OK && responseCode == 200);
}
*/
