#include "Join.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"

/**
 * @brief Sends a properly formatted numeric reply to the client.
 *
 * The reply follows the IRC format:
 *   :<server> <numeric> <nick> <args> :<message>
 *
 * @param server Pointer to the Server.
 * @param fd The client's file descriptor.
 * @param numeric The numeric code (e.g. "473").
 * @param args Additional arguments (usually the channel name).
 * @param message The message text.
 */
static void sendNumericWithServer(Server* server, int fd, const std::string &numeric,
                                  const std::string &args, const std::string &message) {
    std::string reply = ":" + server->getServerName() + " " + numeric + " " +
                        server->getClients()[fd]->getNickname() + " " + args +
                        " :" + message + "\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}

/**
 * @brief Validates the channel name.
 *
 * Channel names must start with '#' according to IRC rules.
 *
 * @param server Pointer to the Server.
 * @param fd The client's file descriptor.
 * @param channelName The channel name to validate.
 * @return true if the name is valid, false otherwise.
 */
static bool validateChannelName(Server* server, int fd, const std::string &channelName) {
    if (channelName.empty() || channelName[0] != '#') {
        sendNumericWithServer(server, fd, "479", channelName,
                              "Illegal channel name. Channel names must start with '#'");
        return false;
    }
    return true;
}

/**
 * @brief Broadcasts the JOIN message to all clients in the channel.
 *
 * Sends the JOIN event (in IRC format) both to the joining client and to all
 * other members of the channel.
 *
 * @param server Pointer to the Server.
 * @param channelName The channel name.
 * @param prefix The prefix string in the format ":Nick!user@host".
 * @param joiningFd The client's file descriptor.
 */
static void broadcastJoin(Server *server, const std::string &channelName,
                          const std::string &prefix, int joiningFd) {
    std::string joinMsg = prefix + " JOIN " + channelName + "\r\n";
    // Send JOIN event to the joining client.
    send(joiningFd, joinMsg.c_str(), joinMsg.size(), 0);
    // Send JOIN event to all other members in the channel.
    Channel &chan = server->getChannels()[channelName];
    for (int clientFd : chan.getClients()) {
        if (clientFd != joiningFd) {
            send(clientFd, joinMsg.c_str(), joinMsg.size(), 0);
        }
    }
}

/**
 * @brief Sends the NAMES list replies (353 and 366) to the client.
 *
 * This informs the client of the list of users in the channel.
 *
 * @param server Pointer to the Server.
 * @param fd The client's file descriptor.
 * @param channelName The channel name.
 */
static void sendNamesReply(Server *server, int fd, const std::string &channelName) {
    Channel &chan = server->getChannels()[channelName];
    std::string nick = server->getClients()[fd]->getNickname();
    
    // Build the 353 reply (names list).
    std::string names = "353 " + nick + " = " + channelName + " :";
    for (int cli_fd : chan.getClients()) {
        if (chan.isOperator(cli_fd)) {
            names += "@";
        }
        names += server->getClients()[cli_fd]->getNickname() + " ";
    }
    names += "\r\n";
    // Prepend the server prefix.
    std::string fullNames = ":" + server->getServerName() + " " + names;
    send(fd, fullNames.c_str(), fullNames.size(), 0);
    
    // Send the 366 reply indicating end of NAMES list.
    std::string endNames = "366 " + nick + " " + channelName + " :End of /NAMES list\r\n";
    std::string fullEndNames = ":" + server->getServerName() + " " + endNames;
    send(fd, fullEndNames.c_str(), fullEndNames.size(), 0);
}

/**
 * @brief Sends the channel topic (332 reply) to the client, if set.
 *
 * @param server Pointer to the Server.
 * @param fd The client's file descriptor.
 * @param channelName The channel name.
 */
static void sendTopicReply(Server *server, int fd, const std::string &channelName) {
    Channel &chan = server->getChannels()[channelName];
    if (!chan.getTopic().empty()) {
        std::string topicMsg = "332 " + server->getClients()[fd]->getNickname() +
                               " " + channelName + " :" + chan.getTopic() + "\r\n";
        std::string fullTopic = ":" + server->getServerName() + " " + topicMsg;
        send(fd, fullTopic.c_str(), fullTopic.size(), 0);
    }
}

/**
 * @brief Handles the JOIN command from a client.
 *
 * Processes the JOIN command. Checks that the client is registered, validates
 * the channel name, and then:
 * - If the channel exists and has invite-only mode (+i) enabled, verifies that the
 *   client is either an operator or invited. If not, sends error 473.
 * - If the channel does not exist, creates it (standard IRC behavior).
 * - Checks user limit (+l) and channel key (+k) restrictions.
 * - If all checks pass, adds the client to the channel, broadcasts the JOIN event,
 *   and sends the names list and topic.
 *
 * @param server Pointer to the Server.
 * @param fd The client's file descriptor.
 * @param tokens The tokenized JOIN command (expected tokens: "JOIN", channel, [key]).
 * @param command The full JOIN command string.
 */
void handleJoinCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& command)
{
    (void)command; // Unused
    
    // Check that the client is fully registered.
    if (server->getClients()[fd]->authState != AUTH_REGISTERED) {
        sendNumericWithServer(server, fd, "451", "", "You have not registered");
        return;
    }
    
    // Verify that the command has at least one parameter (channel name).
    if (tokens.size() < 2) {
        sendNumericWithServer(server, fd, "461", "JOIN", "Not enough parameters");
        return;
    }
    
    std::string channelName = tokens[1];
    if (!validateChannelName(server, fd, channelName))
        return;
    
    // Find the channel in the server's channel map.
    auto &channels = server->getChannels();
    auto it = channels.find(channelName);
    
    // If the channel exists and the client is already in it.
    if (it != channels.end() && it->second.hasClient(fd)) {
        sendNumericWithServer(server, fd, "443", channelName, "You are already in the channel");
        return;
    }
    
    bool isFirstUser = false;
    // If the channel does not exist, create it.
    if (it == channels.end()) {
        auto result = channels.emplace(channelName, Channel(channelName));
        if (!result.second) {
            std::cerr << "Failed to create channel: " << channelName << std::endl;
            return;
        }
        it = result.first;
        isFirstUser = true;
    }
    
    Channel &chan = it->second;
    
    // --- Invite-only check (+i) ---
    // If the channel is invite-only, the client must be either an operator or invited.
    if (chan.isInviteOnly() && !chan.isOperator(fd) && !chan.isInvited(fd)) {
        sendNumericWithServer(server, fd, "473", channelName, "Cannot join channel (+i mode set)");
        return;
    }
    
    // --- User limit check (+l) ---
    if (chan.getUserLimit() > 0 &&
        static_cast<int>(chan.getClients().size()) >= chan.getUserLimit()) {
        sendNumericWithServer(server, fd, "471", channelName, "Channel is full");
        return;
    }
    
    // --- Channel key check (+k) ---
    if (chan.hasMode('k')) {
        if (tokens.size() < 3 || chan.getChannelKey() != tokens[2]) {
            sendNumericWithServer(server, fd, "475", channelName, "Cannot join channel (+k mode set)");
            return;
        }
    }
    
    // Add the client to the channel.
    chan.addClient(fd);
    if (chan.isInvited(fd)) {
        chan.removeInvite(fd);
    }
    
    // If this is the first user in the channel, make them an operator.
    if (isFirstUser) {
        chan.addOperator(fd);
        std::string opMsg = server->getClients()[fd]->getNickname() + " MODE " +
                            channelName + " +o " +
                            server->getClients()[fd]->getNickname();
        sendNumericWithServer(server, fd, "MODE", channelName, opMsg);
    }
    
    // Build the prefix string in the format: ":Nick!user@host"
    Client* client = server->getClients()[fd].get();
    std::string nick = client->getNickname();
    std::string user = client->getUsername();
    if (user.empty()) {
        user = "unknown";
    }
    std::string host = client->getHost();
    std::string prefix = ":" + nick + "!" + user + "@" + host;
    
    // Broadcast the JOIN event to all members in the channel.
    broadcastJoin(server, channelName, prefix, fd);
    
    // Send the NAMES list replies (353 and 366) to the client.
    sendNamesReply(server, fd, channelName);
    
    // Send the channel topic (if any) to the client.
    sendTopicReply(server, fd, channelName);
}
