#include "Part.hpp"
#include "../include/Server.hpp"
#include <string>

void handlePartCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& /*command*/) {
    // Check if the client is fully registered
    if (server->_clients[fd]->authState != AUTH_REGISTERED) {
        std::string reply = "451 :You have not registered\r\n"; // Error: Client not registered
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Ensure the command has enough parameters (at least the channel name)
    if (tokens.size() < 2) {
        std::string reply = "461 PART :Not enough parameters\r\n"; // Error: Missing parameters
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string channelName = tokens[1]; // Extract the channel name from the command

    // Find the channel in the server's channel map
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end()) {
        std::string reply = "403 " + channelName + " :No such channel\r\n"; // Error: Channel does not exist
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Check if the client is part of the channel
    if (!it->second.hasClient(fd)) {
        std::string reply = "442 " + channelName + " :You're not on that channel\r\n"; // Error: Client not in channel
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Check if a part message was provided; if not, use the default "Leaving" message
    std::string partMessage = tokens.size() > 2 ? tokens[2] : "Leaving";

    // Construct the PART message to be broadcast to all channel members
    std::string fullPartMessage = ":" + server->_clients[fd]->nickname + " PART " + channelName + " :" + partMessage + "\r\n";

    // Notify all clients in the channel about the PART event
    for (int cli_fd : it->second.getClients()) {
        send(cli_fd, fullPartMessage.c_str(), fullPartMessage.size(), 0);
    }

    // Remove the client from the channel
    it->second.removeClient(fd);

    // If the channel becomes empty after the client leaves, delete the channel
    if (it->second.getClients().empty()) {
        server->_channels.erase(it); // Remove the channel from the server
    }
}
