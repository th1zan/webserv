# Questions for Webserv

 3. handling ssl & https? open questions

 4. why have the sockets be  SO_REUSEADDR and not SO_REUSEPORT ?  (usefull to have multiple servers on the same port) do we need our servers to be listening on the same port?

 5. how do we handle multiple concurrent & conflicting requests to different servers?  
 open question


Todo:
do (issues):
    x github branchs
    x check that my server runs
    understand what they mean by:
        ∗Do you wonder what a CGI is?
        ∗Because you won’t call the CGI directly, use the full path as PATH_INFO.
            PATH_INFO is the route information, so when coding my scripts i need to know what paths we will use and output the responses.
        ∗Just remember that, for chunked request, your server needs to unchunk
        it, the CGI will expect EOF as end of the body.

        ∗Same things for the output of the CGI. If no content_length is returned
        from the CGI, EOF will mark the end of the returned data.
        ∗Your program should call the CGI with the file requested as first argument.
        ∗The CGI should be run in the correct directory for relative path file access
    answer questions:
        what are our paths
        when do we unchunk?
    implement http methods in python:
        - GET
        - POST
    implement a 

read:
    location understand the struct
    do we parse and use request headers
    how do we write our response error code
    check the python scripts we want and how to code them
    check if js and how to generate the 