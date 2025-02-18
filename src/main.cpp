#include "../include/Server.hpp"
#include <iostream>
#include <cstdlib>


/**
 * @brief Entry point for the IRC server.
 *
 * This function initializes the server with the specified port and password,
 * then starts the server loop.
 *
 * Usage:
 *   ./ircserv <port> <password>
 *
 * The port must be a valid number in the range 1024-65535.
 * If "port" is specified instead of a number, the default port 6667 is used.
 *
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 * @return EXIT_SUCCESS if the server runs successfully, EXIT_FAILURE otherwise.
 */
int main(int argc, char* argv[])
{
    // Ensure correct usage.
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return EXIT_FAILURE;
    }

    int port;
    
    // Allow "port" as an argument to default to 6667.
    if (std::string(argv[1]) == "port") {
        port = 6667;
    } 
    else {
        try {
            port = std::stoi(argv[1]);  // Convert the string to an integer.
            
            // Validate the port number.
            if (port < 1024 || port > 65535) {
                std::cerr << "Error: Port must be in the range 1024-65535.\n";
                return EXIT_FAILURE;
            }
        } 
        catch (const std::exception&) {
            std::cerr << "Invalid port number.\n";
            return EXIT_FAILURE;
        }
    }

    std::string password = argv[2];  // Store the provided password.

    try {
        Server server(port, password);  // Initialize the server.
        server.run();                   // Start the server loop.
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

