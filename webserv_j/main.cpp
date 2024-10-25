#include "webserver.hpp"

int main() {
    WebServer server;
    server.startServer("config.conf");
    server.stopServer();
    return 0;
}
