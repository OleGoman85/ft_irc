#include "Kick.hpp"
#include "../include/Server.hpp"
#include <string>

/**
 * @brief Handles the KICK command from a client.
 *
 * This function processes the KICK command, which is used to remove a target user from a specified channel.
 * It first verifies that the sender is fully registered, then checks whether enough parameters were provided.
 * Next, it attempts to locate the specified channel and finds the target user by nickname. If the target user
 * is found as a member of the channel, they are removed. A KICK notification is then broadcast to all remaining
 * channel members. If the target user is not found, an error message is sent back to the sender.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the KICK command.
 * @param tokens A vector of strings representing the tokenized command (expected: "KICK", <channelName>, <targetNick>).
 * @param command The complete command string (unused in this implementation).
 */
void handleKickCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& /*command*/) {
    // Check if the client (sender) is fully registered.
    if (server->_clients[fd]->authState != AUTH_REGISTERED) {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Verify that enough parameters are provided: KICK <channel> <targetNick>
    if (tokens.size() < 3) {
        std::string reply = "461 KICK :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    std::string channelName = tokens[1];
    std::string targetNick = tokens[2];
    
    // Find the specified channel.
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end()) 
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    bool found = false;
    // Search for the target user in the channel.
    for (int cli_fd : it->second.getClients()) {
        auto clientIt = server->_clients.find(cli_fd);
        if (clientIt == server->_clients.end()) {
            continue; // Skip if the client is no longer there
        }

        if (clientIt->second->nickname == targetNick) {
            it->second.removeClient(cli_fd);
            if (it->second.getClients().empty()) {
                server->_channels.erase(it); // Delete the channel if it is empty
            }

            std::string kickMsg = ":" + server->_clients[fd]->nickname + " KICK " + channelName + " " + targetNick + " :Kicked\r\n";
            server->broadcastMessage(kickMsg, fd);
            found = true;
            break;
        }
    }
    // If no such target user is found, send an error message.
    if (!found) {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
    }
}
