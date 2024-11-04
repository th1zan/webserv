/**
 * @file Service.cpp
 * @brief Implementation of the Service class responsible for server management and client connection handling.
 */

#include "Service.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "defines.hpp"

/**
 * @brief Constructs a Service object, sets up signal handling, and parses the configuration file.
 * @param argc Argument count from command line.
 * @param argv Argument values from command line.
 */
Service::Service(int argc, char **argv) {
    printInfo(START_MSG, GREEN);

    // Intercepts signals for socket handling and interruption.
    std::signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE errors for sockets.
    std::signal(SIGINT, signalHandler);

    // Instantiate Parser to process the configuration file.
    Parser input(argc, argv);

    this->_serversVector = input.getServersVector();
    this->_defaultServers = 1; // Initially set to 1 default server.
}

/**
 * @brief Destructor for Service, closes all sockets in the polling vector.
 */
Service::~Service() {
    for (size_t i = 0; i < this->_pollingFdVector.size(); i++)
        close(this->_pollingFdVector.at(i).fd);
    printInfo(END_MSG, GREEN);
}

/**
 * @brief Initializes the server setup, binds addresses, and adds sockets to the polling vector.
 */
void Service::setup() {
    this->_initServiceInfo();

    for (std::vector<Server>::iterator server = this->_serversVector.begin(); server != this->_serversVector.end(); server++) {
        server->printServers();
        this->_getSetupInfo(server);
        this->printServiceInfo();
        this->_setReuseableAddress();
        this->_convertHostToAddress();
        this->_bindAddressToSocket();
        this->_setSocketListening();
        this->_addSocketToPollSockVec();
        this->_resetTmpServiceInfo();
    }
}

/**
 * @brief Initializes temporary ServiceInfo parameters for server setup.
 */
void Service::_initServiceInfo() {
    std::memset(&this->_tmpServiceInfo.parameters, 0, sizeof(this->_tmpServiceInfo.parameters));
    this->_tmpServiceInfo.parameters.ai_family = AF_INET;  // IPv4
    this->_tmpServiceInfo.parameters.ai_socktype = SOCK_STREAM; // TCP
    this->_tmpServiceInfo.parameters.ai_protocol = IPPROTO_TCP; // TCP
    this->_tmpServiceInfo.address = NULL;
}

/**
 * @brief Retrieves setup information from the server and stores it in temporary variables.
 * @param server Iterator pointing to the current Server.
 */
void Service::_getSetupInfo(std::vector<Server>::iterator server) {
    server->createSocket();
    this->_tmpServiceInfo.listeningSocketFd = server->getSocket();
    this->_tmpServiceInfo.host = server->getHost();
    this->_tmpServiceInfo.port = server->getPort();
    this->_tmpServiceInfo.launch = false;
}

/**
 * @brief Sets the SO_REUSEADDR option on the listening socket to allow reuse of the socket address.
 */
void Service::_setReuseableAddress() {
    int active = 1;
    if (setsockopt(this->_tmpServiceInfo.listeningSocketFd, SOL_SOCKET, SO_REUSEADDR, &active, sizeof(int)) < 0) {
        this->_resetTmpServiceInfo();
        throw std::runtime_error(ERR_SET_SOCKET + std::string(std::strerror(errno)));
    }
}

/**
 * @brief Resets temporary ServiceInfo variables for new configuration.
 */
void Service::_resetTmpServiceInfo() {
    if (this->_tmpServiceInfo.address) {
        freeaddrinfo(this->_tmpServiceInfo.address);
        this->_tmpServiceInfo.address = NULL;
    }
    std::memset(&this->_tmpServiceInfo.parameters, 0, sizeof(this->_tmpServiceInfo.parameters));
    this->_tmpServiceInfo.host.clear();
    this->_tmpServiceInfo.port.clear();
    this->_tmpServiceInfo.pollID = 0;
    this->_tmpServiceInfo.clientID = 0;
    this->_tmpServiceInfo.serverID = 0;
    this->_tmpServiceInfo.listeningSocketFd = 0;
    this->_tmpServiceInfo.mode = 0;
    this->_tmpServiceInfo.connectionSocketFd = 0;
    this->_tmpServiceInfo.launch = false;
}

/**
 * @brief Converts the hostname to an IP address for use with sockets.
 */
void Service::_convertHostToAddress() {
    if (getaddrinfo(this->_tmpServiceInfo.host.c_str(), this->_tmpServiceInfo.port.c_str(), &this->_tmpServiceInfo.parameters, &this->_tmpServiceInfo.address) != 0) {
        this->_resetTmpServiceInfo();
        throw std::runtime_error(ERR_GET_ADDR_INFO + std::string(std::strerror(errno)));
    }
}

/**
 * @brief Binds the server address to the socket.
 */
void Service::_bindAddressToSocket() {
    if (this->_tmpServiceInfo.address) {
        if (bind(this->_tmpServiceInfo.listeningSocketFd, this->_tmpServiceInfo.address->ai_addr, this->_tmpServiceInfo.address->ai_addrlen) < 0) {
            this->_resetTmpServiceInfo();
            throw std::runtime_error(ERR_BIND_SOCKET + std::string(std::strerror(errno)));
        }
    }
}

/**
 * @brief Sets the socket to listening mode.
 */
void Service::_setSocketListening() {
    if (listen(this->_tmpServiceInfo.listeningSocketFd, MAX_PENDING) < 0) {
        this->_resetTmpServiceInfo();
        throw std::runtime_error(ERR_LISTEN_SOCKET + std::string(std::strerror(errno)));
    }
}

/**
 * @brief Adds the configured socket to the polling vector for monitoring.
 */
void Service::_addSocketToPollSockVec() {
    pollfd pollSocket;
    if (this->_tmpServiceInfo.launch == true) {
        pollSocket.fd = this->_tmpServiceInfo.connectionSocketFd;
        pollSocket.events = POLLIN | POLLOUT;
    } else {
        pollSocket.fd = this->_tmpServiceInfo.listeningSocketFd;
        pollSocket.events = POLLIN;
    }
    pollSocket.revents = 0;
    this->_pollingFdVector.push_back(pollSocket);
}

/**
 * @brief Begins the main polling loop to handle client connections and server actions.
 */
void Service::launch() {
    printInfo(LAUNCH_MSG, BLUE);

    while (g_shutdown == false) {
        this->_initPollingVector();
        this->_pollingManager();
    }
}

/**
 * @brief Initializes the polling vector by calling poll on the sockets.
 */
void Service::_initPollingVector() {
    if (poll(this->_pollingFdVector.data(), this->_pollingFdVector.size(), POLL_TIME_OUT) < 0 && g_shutdown == false)
        throw std::runtime_error(ERR_POLL_FAIL + std::string(std::strerror(errno)));
}

/**
 * @brief Manages polling events for each socket, handling client-server interactions.
 */
void Service::_pollingManager() {
    for (size_t i = 0; i < this->_pollingFdVector.size(); i++) {
        this->_resetTmpServiceInfo();
        this->_getLaunchInfo(i);

        if (this->_hasBadRequest())
            continue;

        if (this->_tmpServiceInfo.mode & POLLIN) {
            if (this->_isServerSocket()) {
                this->_acceptConnection();
            } else {
                this->_readDataFromClient();
            }
        } else if (this->_tmpServiceInfo.mode & POLLOUT) {
            this->_sendDataToClient();
        }
    }
}

/**
 * @brief Retrieves launch-specific information for each socket in the polling vector.
 * @param i Index of the socket in the polling vector.
 */
void Service::_getLaunchInfo(int const i) {
    this->_tmpServiceInfo.pollID = i;
    this->_tmpServiceInfo.clientID = i - this->_defaultServers;
    this->_tmpServiceInfo.listeningSocketFd = this->_pollingFdVector.at(i).fd;
    this->_tmpServiceInfo.mode = this->_pollingFdVector.at(i).revents;
    this->_tmpServiceInfo.serverID = this->_getServerIndex();
    this->_tmpServiceInfo.launch = true;
}

/**
 * @brief Checks if the socket is a server socket.
 * @return True if socket belongs to a server, false otherwise.
 */
bool Service::_isServerSocket() {
    for (std::vector<Server>::iterator server = this->_serversVector.begin(); server != this->_serversVector.end(); server++) {
        if (server->getSocket() == this->_tmpServiceInfo.listeningSocketFd)
            return true;
    }
    return false;
}

/**
 * @brief Accepts a new client connection on the server socket.
 */
void Service::_acceptConnection() {
    this->_tmpServiceInfo.connectionSocketFd = accept(this->_tmpServiceInfo.listeningSocketFd, NULL, NULL);
    if (this->_tmpServiceInfo.connectionSocketFd < 0)
        throw std::runtime_error(ERR_ACCEPT_SOCKET);

    fcntl(this->_tmpServiceInfo.connectionSocketFd, F_SETFL, O_NONBLOCK);
    this->_clientVector.push_back(Client(this->_serversVector.at(this->_tmpServiceInfo.serverID), this->_tmpServiceInfo.connectionSocketFd));
    this->_addSocketToPollSockVec();
}

/**
 * @brief Reads data from a client socket into the buffer.
 */
void Service::_readDataFromClient() {
    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(this->_tmpServiceInfo.listeningSocketFd, buffer, BUFFER_SIZE, 0);
    if (bytes <= 0)
        this->_removeClient();
    else
        this->_clientVector.at(this->_tmpServiceInfo.clientID).appendRequest(buffer, bytes);
}

/**
 * @brief Sends response data to the client.
 */
void Service::_sendDataToClient() {
    int bytes = this->_clientVector.at(this->_tmpServiceInfo.clientID).sendResponse();
    if (bytes <= 0)
        this->_removeClient();
}

/**
 * @brief Removes a client and its associated socket from the polling vector.
 */
void Service::_removeClient() {
    close(this->_pollingFdVector.at(this->_tmpServiceInfo.pollID).fd);
    this->_pollingFdVector.erase(this->_pollingFdVector.begin() + this->_tmpServiceInfo.pollID);
    this->_clientVector.erase(this->_clientVector.begin() + this->_tmpServiceInfo.clientID);
}

/**
 * @brief Checks if there is a polling error or if the socket is disconnected.
 * @return True if there is a bad request, false otherwise.
 */
bool Service::_hasBadRequest() {
    return (this->_tmpServiceInfo.mode & POLLHUP || this->_tmpServiceInfo.mode & POLLERR || this->_tmpServiceInfo.mode & POLLNVAL);
}

/**
 * @brief Retrieves the server index for the current socket.
 * @return Server index as integer.
 */
int Service::_getServerIndex() const {
    int i = 0;
    for (std::vector<Server>::const_iterator it = this->_serversVector.begin(); it != this->_serversVector.end(); it++, i++) {
        if (it->getSocket() == this->_tmpServiceInfo.listeningSocketFd)
            return i;
    }
    return 0;
}
