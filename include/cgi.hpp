/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 18:33:27 by zsoltani          #+#    #+#             */
/*   Updated: 2024/11/28 22:10:06 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "defines.hpp"

class CGI {
private:
    std::string _scriptPath;      ///< Full path to the CGI script
    std::string _method;          ///< HTTP request method (e.g., GET, POST)
    std::string _queryString;     ///< Query string for GET requests
    std::string _requestBody;     ///< Request payload (POST body)
    std::string _pathInfo;        ///< Additional path information
    std::string _scriptFileName;  ///< Full script filename
    std::vector<std::string> _environmentVars; ///< CGI environment variables

    void setEnvironment(); ///< Prepares CGI environment variables

public:
    // CGI(const std::string &scriptPath, const std::string &method, const std::string &queryString, 
    //     const std::string &body, const std::string &pathInfo, const std::string &pathTranslated);
	public:
    CGI(const std::string &scriptPath, const std::string &method, 
        const std::string &queryString, const std::string &requestBody,
        const std::string &pathInfo, const std::string &scriptFileName);
    ~CGI();

    std::string execute(); ///< Executes the CGI script and returns its output.
};

#endif
