/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsoltani <zsoltani@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 19:26:05 by zsoltani          #+#    #+#             */
/*   Updated: 2024/12/05 16:07:55 by zsoltani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cgi.hpp"
#include <defines.hpp>
#include <utils.hpp>

CGI::CGI(const std::string &scriptPath, const std::string &method, 
         const std::string &queryString, const std::string &requestBody,
         const std::string &pathInfo, const std::string &scriptFileName)
    : _scriptPath(scriptPath), _method(method), _queryString(queryString), 
      _requestBody(requestBody), _pathInfo(pathInfo), _scriptFileName(scriptFileName) {}

CGI::~CGI() {}

void CGI::setEnvironment()
{
    _environmentVars.push_back("REQUEST_METHOD=" + _method);
    std::cout << "Set REQUEST_METHOD: " << _method << std::endl;

    _environmentVars.push_back("QUERY_STRING=" + _queryString);
    std::cout << "Set QUERY_STRING: " << _queryString << std::endl;

    _environmentVars.push_back("CONTENT_LENGTH=" + to_string(_requestBody.size()));
    std::cout << "Set CONTENT_LENGTH: " << to_string(_requestBody.size()) << std::endl;

    _environmentVars.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
    std::cout << "Set CONTENT_TYPE: application/x-www-form-urlencoded" << std::endl;

    _environmentVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    std::cout << "Set SERVER_PROTOCOL: HTTP/1.1" << std::endl;

    _environmentVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    std::cout << "Set GATEWAY_INTERFACE: CGI/1.1" << std::endl;

    _environmentVars.push_back("PATH_INFO=" + _pathInfo);
    std::cout << "Set PATH_INFO: " << _pathInfo << std::endl;

    _environmentVars.push_back("SCRIPT_FILENAME=" + _scriptFileName);
    std::cout << "Set SCRIPT_FILENAME: " << _scriptFileName << std::endl;

    _environmentVars.push_back(""); // Null-terminate for execve
    std::cout << "Set Null-Terminator for environment variables" << std::endl;
}


std::string CGI::execute()
{
    setEnvironment();

    // Create pipes for communication
    int pipeIn[2], pipeOut[2];
    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
    {
        perror("Pipe creation failed");
        return "Error: Pipe creation failed";
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork failed");
        return "Error: Fork failed";
    }

    if (pid == 0) { // Child process
        // Set up pipes
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(pipeIn[1]);
        close(pipeOut[0]);

        // Prepare environment variables
        std::vector<char*> envVars(_environmentVars.size() + 1, NULL);
        for (size_t i = 0; i < _environmentVars.size(); ++i)
        {
            envVars[i] = const_cast<char*>(_environmentVars[i].c_str());
        }
       
        // Use _scriptPath as the interpreter and _scriptFileName as the script to execute
        char *argv[] = {
            const_cast<char*>(_scriptPath.c_str()),     // Interpreter (e.g., Python)
            const_cast<char*>(_scriptFileName.c_str()), // Script (e.g., age_calculator.py)
            NULL                                        // Null-terminate the argument list
        };

        // Execute the interpreter with the script file as an argument
        execve(_scriptPath.c_str(), argv, envVars.data());

        // If execve fails, log the error and exit
        perror("execve failed");
        exit(1);
    }
    else
    { // Parent process
        close(pipeIn[0]);
        close(pipeOut[1]);

        if (_method == "POST" && !_requestBody.empty())
        {
           if (write(pipeIn[1], _requestBody.c_str(), _requestBody.size()) == -1) {
                perror("Failed to write to pipe");
            }
        }
        close(pipeIn[1]);

        char buffer[1024];
        std::ostringstream output;
        ssize_t bytesRead;

        while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0)
            output.write(buffer, bytesRead);
        close(pipeOut[0]);

        int status;
        waitpid(pid, &status, 0);

        // Check the child's exit status and return the output or an error message
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
        // Separate headers and body
            std::string fullOutput = output.str();
            size_t headerEnd = fullOutput.find("\r\n\r\n");
            if (headerEnd != std::string::npos)
            {
                return fullOutput.substr(headerEnd + 4); // Skip headers
            }
            return fullOutput; // If no headers, return everything
        }
        else
            return "Error: CGI script execution failed";
    }
}
