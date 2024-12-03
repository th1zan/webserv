#include <defines.hpp>


/* Todo:
    I need to handle:
        if there is no script in that cgi directory
        if the script is not executable
        the output file of the script (which should be the http response)


	I need for my structure:
        - A filename
        - A upload directory
        - A scriptpath that has been set during the creation of the server block?
    
*/

/* 
    Can i:
        - Run a CGI in this block
            -[x] is the CGI extension valid
            -[x] is the CGI path valid
            -[ ] check the allowed_methods for the CGI
            -[x] is the directory for the cgi valid
        - 

*/

class Script
{
private:
    std::string _scriptPath;
    std::string _uploadDirectory;
    std::string _fileName;
    std::string _method;
    std::string _request;
    size_t _contentLength;
    char** _envp;
    char** _argv;
public:
    Script::Script::Script(const std::string request, const std::string filename, const std::string method, const std::string CGIDirectory);
    ~Script() {};
    
    char** _setArgvEnv();

    std::string _getUploadDirectory(); //TODO
    std::string _getScriptPath(std::string CGIDirectory, std::string method);
    std::string _getEnv();//TODO

    void _run();
};