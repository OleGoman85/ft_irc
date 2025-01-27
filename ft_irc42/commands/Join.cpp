#include "Join.hpp"
#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include <sys/socket.h>
#include <iostream>
#include <string>

/**
 * @brief Handles the JOIN command from a client.
 *
 * This function processes the JOIN command, which allows a client to join a specified channel.
 * It verifies that the client is fully registered, checks that the required parameters are provided,
 * creates the channel if it does not exist, and performs several validations:
 * - If the channel is invite-only, only permitted clients (e.g., channel operators) are allowed to join.
 * - If a user limit is set, the channel cannot be joined when the limit is reached.
 *
 * If all checks pass, the client is added to the channel, and a JOIN notification is sent to all other
 * members of the channel.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the JOIN command.
 * @param tokens A vector of tokens parsed from the JOIN command; expected tokens are: "JOIN" and <channelName>.
 * @param command The complete JOIN command string (unused in this implementation).
 */
void handleJoinCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) {
    (void)command; // Suppress unused parameter warning

    // Check if the client is fully registered.
    if (server->_clients[fd]->authState != AUTH_REGISTERED) {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Ensure that the command has enough parameters (minimum: JOIN <channel>).
    if (tokens.size() < 2) {
        std::string reply = "461 JOIN :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Extract the channel name from the command.
    std::string channelName = tokens[1];
    
    // Look for the channel in the server's channel map.
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end()) {
        // If the channel does not exist, create a new one.
        auto emplaceResult = server->_channels.emplace(channelName, Channel(channelName));
        if (!emplaceResult.second) {
            std::cerr << "Failed to create channel: " << channelName << std::endl;
            return;
        }
        it = emplaceResult.first;
    }
    
    // Check if the channel is set to invite-only and if the client is permitted (e.g., is an operator).
    if (it->second.isInviteOnly() && !it->second.isOperator(fd)) {
        std::string reply = "473 " + channelName + " :Cannot join channel (+i mode set)\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Check if a user limit is set and whether the channel is already full.
    if (it->second.getUserLimit() > 0 && static_cast<int>(it->second.getClients().size()) >= it->second.getUserLimit()) {
        std::string reply = "471 " + channelName + " :Channel is full\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // (Optional) If the channel has a key (mode 'k') set, the key should be verified here.
    
    // Add the client to the channel.
    it->second.addClient(fd);
    
    // Compose a JOIN message to notify all other members of the channel.
    std::string joinMsg = ":" + server->_clients[fd]->nickname + " JOIN " + channelName + "\r\n";
    for (int cli_fd : it->second.getClients()) {
        // Optionally, do not send the JOIN message to the joining client.
        if (cli_fd != fd)
            send(cli_fd, joinMsg.c_str(), joinMsg.size(), 0);
    }
}
