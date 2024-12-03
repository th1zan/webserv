#include "Script.hpp"
#include "Client.hpp"

Script::Script(const std::string request, const std::string filename, const std::string method, const std::string CGIDirectory)
{
    this->_fileName = filename;
    this->_request = request;
    this->_contentLength = request.size();
    this->_uploadDirectory = this->_getUploadDirectory();
    this->_scriptPath = CGIDirectory + "/" + method + ".py";
    this->_envp = this->_setArgvEnv();
    this->_run();
}

Script::~Script()
{
	remove(CGI_OUTPUT_FILE);
	if (this->_argv)
	{
		for (size_t i = 0; this->_argv[i]; i++)
			free(this->_argv[i]);
		delete [] this->_argv;
	}

	if (this->_envp)
	{
		for (size_t i = 0; this->_envp[i]; i++)
			free(this->_envp[i]);
		delete [] this->_envp;
	}
}

char** Script::_setArgvEnv()
{
    char** envp = new char*[5];

    this->_argv = new char*[4];
    this->_argv[0] = strdup(PYTHON_VERSION);
    this->_argv[1] = strdup(this->_scriptPath.c_str());
    this->_argv[2] = strdup(this->_fileName.c_str());
    return envp;
}

/* this function returns the HTML*/
void Script::_run() {
    int fd = open(CGI_OUTPUT_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
        throw std::runtime_error("open() failed");

    int pipefd[2];
    if (pipe(pipefd) == -1)
        throw std::runtime_error("pipe() failed");

    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error("fork() failed");

    if (pid == 0)
    {
        // Child process
        close(pipefd[0]);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execve(PYTHON_VERSION, this->_argv, this->_envp);
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        close(pipefd[1]);
        char buffer[4096];
        std::string output;
        ssize_t bytesRead;
        while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytesRead);
        }
        close(pipefd[0]);
        close(fd);
        int status;
        waitpid(pid, &status, 0);
    }
}