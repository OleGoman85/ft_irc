
#include "Invite.hpp"
#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include <string>

/**
 * @brief Handles the INVITE command.
 *
 * This function processes an INVITE command sent by a client. It verifies that the
 * sender is fully registered, checks that the correct parameters are provided,
 * searches for the target client and the specified channel, and then sends an invitation.
 * Additionally, if the target client is not already a member of the channel,
 * it is automatically added to the channel and a JOIN notification is sent to all members.
 *
 * @param server Pointer to the Server object that manages the IRC server.
 * @param fd The file descriptor of the client sending the INVITE command.
 * @param tokens The tokenized command arguments (expected to be: INVITE <targetNick> <channel>).
 * @param command The complete command string (unused in this implementation).
 */
void handleInviteCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& /*command*/) 
{
    // Check that the client (sender) is fully registered.
    if (server->_clients[fd]->authState != AUTH_REGISTERED) 
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Ensure there are enough parameters: INVITE <targetNick> <channel>
    if (tokens.size() < 3) {
        std::string reply = "461 INVITE :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Extract target nickname and channel name.
    std::string targetNick = tokens[1];
    std::string channelName = tokens[2];
    
    // Find the channel in the server's channel map.
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end()) {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    int targetFd = -1;
    // Search for the client with the specified nickname.
    for (const auto& pair : server->_clients) {
        if (pair.second->nickname == targetNick) {
            targetFd = pair.first;
            break;
        }
    }
    
    if (targetFd == -1) {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Send an invitation message to the target client.
    std::string inviteMsg = ":" + server->_clients[fd]->nickname + " INVITE " + targetNick + " " + channelName + "\r\n";
    send(targetFd, inviteMsg.c_str(), inviteMsg.size(), 0);

    // If the target client is not already in the channel, add it automatically.
    // (If the client is already a member, addClient() will prevent duplicate entries.)
    if (!it->second.hasClient(targetFd)) {
        it->second.addClient(targetFd);
        
        // Notify all members of the channel that the target client has joined.
        std::string joinMsg = ":" + server->_clients[targetFd]->nickname + " JOIN " + channelName + "\r\n";
        const std::vector<int>& channelClients = it->second.getClients();
        for (size_t i = 0; i < channelClients.size(); ++i) {
            if (channelClients[i] != targetFd) {
                send(channelClients[i], joinMsg.c_str(), joinMsg.size(), 0);
            }
        }
    }
}
