#include "Server.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

static void handleSignal(int signum)
{
    if (signum == SIGINT) {
        std::cerr << "\nCaught SIGINT! Shutting down...\n";
        Server::requestShutdown();

    } else if (signum == SIGQUIT) {
        std::cerr << "\nCaught SIGQUIT! Shutting down...\n";
        Server::requestShutdown();
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return EXIT_FAILURE;
    }

    std::signal(SIGINT, handleSignal);
    std::signal(SIGQUIT, handleSignal);

    int port;
    try {
        port = std::stoi(argv[1]);
        if (port < 1024 || port > 65535) {
            std::cerr << "Error: Port must be in the range 1024-65535.\n";
            return EXIT_FAILURE;
        }
    } catch (...) {
        std::cerr << "Invalid port number.\n";
        return EXIT_FAILURE;
    }
    std::string password = argv[2];

    try {
        Server server(port, password);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
