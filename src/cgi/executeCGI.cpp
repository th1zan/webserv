//#include <webserv.hpp>
#include <iostream>
#include <csignal>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <exception>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
//#include <regex>
#include <fcntl.h>
#include <stdexcept>
#include <cctype>
#include <unistd.h>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <dirent.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <limits>


typedef struct s_cgi {
	std::string method;
	std::string body;
	std::string fileName;
	std::string scriptPath;
} t_cgi;


std::string executeCGI(t_cgi body);
void scriptCall(t_cgi body);
char **setEnv(t_cgi body);

int main(int argc, char **argv) {
	if (argc != 5) {
		std::cerr << "Usage: " << argv[0] << " <method> <body> <fileName> <scriptPath>" << std::endl;
		return EXIT_FAILURE;
	}

	t_cgi body = {
		.method = argv[1],
		.body = argv[2],
		.fileName = argv[3],
		.scriptPath = argv[4]
	};

	try {
		std::string resp = executeCGI(body);
		std::cout << resp;
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

std::string executeCGI(t_cgi body) {
    std::string resp;
    int pipe_fd[2];
    
    // Create a pipe to capture the CGI output
    if (pipe(pipe_fd) == -1) {
        throw std::runtime_error("Failed to create pipe");
    }
    
    pid_t PID = fork();
    if (PID == -1) {
        throw std::runtime_error("System error: Failed to fork");
    } else if (PID == 0) {
        // Child process
        close(pipe_fd[0]); // Close the read end of the pipe
        
        // Redirect stdout to the write end of the pipe
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2 failed");
            exit(EXIT_FAILURE);
        }
        
        close(pipe_fd[1]); // Close the write end after duplication
		scriptCall(body);
        exit(EXIT_SUCCESS); // Exit the child process
    } else {
        // Parent process
        close(pipe_fd[1]); // Close the write end of the pipe

        // Set a timeout for the child process
        int status;
        int timeout = 5; // Timeout in seconds
        pid_t result = waitpid(PID, &status, WNOHANG);
        
        for (int i = 0; i < timeout && result == 0; ++i) {
            sleep(1);
            result = waitpid(PID, &status, WNOHANG);
        }

        if (result == 0) {
            // Timeout expired; kill the process
            kill(PID, SIGKILL);
            throw std::runtime_error("CGI process timeout");
        } else if (result == -1) {
            throw std::runtime_error("Error waiting for CGI process");
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("CGI process failed");
        }

        // Read the output from the child process
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            resp += buffer;
        }

        close(pipe_fd[0]); // Close the read end
    }

    return resp;
}

void scriptCall(t_cgi body) {
	const char* args[] = {body.method.c_str(),body.body.c_str(), NULL};

	if (execve(body.scriptPath.c_str(), (char *const *)args, body.getEnv()) == -1) {
        perror("execve failed");
    }
}