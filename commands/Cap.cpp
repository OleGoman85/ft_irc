#include "Cap.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include <sstream>
#include <string>
#include <cctype>
#include <sys/socket.h>

// Define the list of server capabilities (can be extended as needed)
static const std::string CAPABILITIES = "multi-prefix";

/**
 * @brief Handles the CAP (capabilities) command from the client.
 *
 * This function processes various CAP subcommands:
 * - LS: Lists the server capabilities.
 * - REQ: Requests specific capabilities.
 * - LIST: Lists currently active capabilities.
 * - CLEAR: Clears (resets) active capabilities.
 * - END: Ends the CAP negotiation (typically no response is needed).
 *
 * @param server Pointer to the Server object (unused in this simple implementation).
 * @param fd File descriptor of the client.
 * @param tokens A vector containing the tokenized command arguments.
 * @param command The full command string from the client (unused here).
 */
void handleCapCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) 
{   
    // Unused parameters are explicitly ignored to suppress compiler warnings.
    (void)server;
    (void)command;

    // Check if a subcommand is provided; if not, return an error.
    if (tokens.size() < 2) {
        std::string reply = "461 CAP :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Convert the provided subcommand to uppercase for case-insensitive matching.
    std::string subCommand = tokens[1];
    for (auto & ch : subCommand)
        ch = std::toupper(static_cast<unsigned char>(ch));

    if (subCommand == "LS") {
        // CAP LS: Return the list of server capabilities.
        std::ostringstream oss;
        oss << "CAP * LS :" << CAPABILITIES << "\r\n";
        std::string reply = oss.str();
        send(fd, reply.c_str(), reply.size(), 0);
    }
    else if (subCommand == "REQ") {
        // CAP REQ: Handle a request for capabilities.
        // The requested capabilities should be provided as the third parameter.
        if (tokens.size() < 3) {
            std::string reply = "461 CAP REQ :Not enough parameters\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        // For simplicity, immediately acknowledge the requested capabilities.
        std::ostringstream oss;
        oss << "CAP * ACK :" << tokens[2] << "\r\n";
        std::string reply = oss.str();
        send(fd, reply.c_str(), reply.size(), 0);
    }
    else if (subCommand == "LIST") {
        // CAP LIST: Return the list of currently active capabilities.
        std::ostringstream oss;
        oss << "CAP * LIST :" << CAPABILITIES << "\r\n";
        std::string reply = oss.str();
        send(fd, reply.c_str(), reply.size(), 0);
    }
    else if (subCommand == "CLEAR") {
        // CAP CLEAR: Clear (reset) the active capabilities.
        std::string reply = "CAP * ACK :\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
    }
    else if (subCommand == "END") {
        // CAP END: End the capability negotiation.
        // Typically no reply is needed here. Any cleanup actions could be performed if required.
    }
    else {
        // If the subcommand is not recognized, return an error indicating an unknown CAP subcommand.
        std::ostringstream oss;
        oss << "421 CAP " << subCommand << " :Unknown CAP subcommand\r\n";
        std::string reply = oss.str();
        send(fd, reply.c_str(), reply.size(), 0);
    }
}
