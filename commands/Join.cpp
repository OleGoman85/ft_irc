#include "Join.hpp"

#include <sys/socket.h>

#include <iostream>
#include <string>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

/**
 * @brief Handles the JOIN command from a client.
 *
 * This function processes the JOIN command, which allows a client to join a
 * specified channel. It verifies that the client is fully registered, checks
 * that the required parameters are provided, creates the channel if it does not
 * exist, and performs several validations:
 * - If the channel is invite-only, only permitted clients (e.g., channel
 * operators) are allowed to join.
 * - If a user limit is set, the channel cannot be joined when the limit is
 * reached.
 *
 * If all checks pass, the client is added to the channel, and a JOIN
 * notification is sent to all other members of the channel.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the JOIN command.
 * @param tokens A vector of tokens parsed from the JOIN command; expected
 * tokens are: "JOIN" and <channelName>.
 * @param command The complete JOIN command string (unused in this
 * implementation).
 */
void handleJoinCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string&              command)
{
    (void)command;  // Suppress unused parameter warning

    // Check if the client is fully registered.
    if (server->_clients[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Ensure that the command has enough parameters (minimum: JOIN <channel>).
    if (tokens.size() < 2)
    {
        std::string reply = "461 JOIN :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Extract the channel name from the command.
    std::string channelName = tokens[1];

    // Look for the channel in the server's channel map.
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end())
    {
        // If the channel does not exist, create a new one.
        auto emplaceResult =
            server->_channels.emplace(channelName, Channel(channelName));
        if (!emplaceResult.second)
        {
            std::cerr << "Failed to create channel: " << channelName
                      << std::endl;
            return;
        }
        it = emplaceResult.first;
    }

    // Check if the channel is set to invite-only and if the client is permitted
    // (e.g., is an operator).
    if (it->second.isInviteOnly() && !it->second.isOperator(fd))
    {
        std::string reply =
            "473 " + channelName + " :Cannot join channel (+i mode set)\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Check if a user limit is set and whether the channel is already full.
    if (it->second.getUserLimit() > 0 &&
        static_cast<int>(it->second.getClients().size()) >=
            it->second.getUserLimit())
    {
        std::string reply = "471 " + channelName + " :Channel is full\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // (Optional) If the channel has a key (mode 'k') set, the key should be
    // verified here.

    // Add the client to the channel.
    it->second.addClient(fd);

    // Compose a JOIN message to notify all other members of the channel.
    std::string joinMsg =
        ":" + server->_clients[fd]->nickname + " JOIN " + channelName + "\r\n";
    for (int cli_fd : it->second.getClients())
    {
        // Optionally, do not send the JOIN message to the joining client.
        if (cli_fd != fd) send(cli_fd, joinMsg.c_str(), joinMsg.size(), 0);
    }
}

// #include <stdexcept>
// #include <iostream>
// #include <vector>
// #include <string>
// #include <map>
// #include "../include/Server.hpp"
// #include "../include/Channel.hpp"

// // Helper: sends an error to a client
// void sendErrorMessage(int fd, const std::string& message) {
//     send(fd, message.c_str(), message.size(), 0);
// }

// // Check if the client is fully registered
// bool isClientRegistered(Server* server, int fd) {
//     if (server->_clients[fd]->authState != AUTH_REGISTERED) {
//         sendErrorMessage(fd, "451 :You have not registered\r\n");
//         return false;
//     }
//     return true;
// }

// // Validate "JOIN <channel> [<key>]"
// bool isJoinCommandValid(int fd, const std::vector<std::string>& tokens) {
//     if (tokens.size() < 2) {
//         sendErrorMessage(fd, "461 JOIN :Not enough parameters\r\n");
//         return false;
//     }
//     return true;
// }

// // Create or find channel
// Channel& getOrCreateChannel(Server* server, const std::string& channelName) {
//     auto it = server->_channels.find(channelName);
//     if (it == server->_channels.end()) {
//         auto res = server->_channels.emplace(channelName,
//         Channel(channelName)); if (!res.second) {
//             std::cerr << "Failed to create channel: " << channelName <<
//             std::endl; throw std::runtime_error("Channel creation failed");
//         }
//         it = res.first;
//     }
//     return it->second;
// }

// // Check invite-only, limit, etc. (but not the +k key yet)
// bool basicChannelChecks(int fd, Channel& channel) {
//     if (channel.isInviteOnly() && !channel.isOperator(fd)) {
//         sendErrorMessage(fd, "473 " + channel.getName() + " :Cannot join
//         channel (+i)\r\n"); return false;
//     }
//     if (channel.getUserLimit() > 0 && (int)channel.getClients().size() >=
//     channel.getUserLimit()) {
//         sendErrorMessage(fd, "471 " + channel.getName() + " :Channel is
//         full\r\n"); return false;
//     }
//     return true;
// }

// /**
//  * Notify all existing channel members that a new user has joined.
//  * Usually: ":<nick> JOIN <channel>\r\n"
//  */
// void notifyChannelMembers(Server* server, int fd, Channel& channel) {
//     std::string joinMsg = ":" + server->_clients[fd]->nickname +
//                           " JOIN " + channel.getName() + "\r\n";

//     for (int cli_fd : channel.getClients()) {
//         // send to everyone (including the newcomer)
//         send(cli_fd, joinMsg.c_str(), joinMsg.size(), 0);
//     }
// }

// /**
//  * Send 353/366 (NAME list, end of NAME list) to the newly joined user.
//  */
// void sendNameReply(int fd, Channel& channel, Server* server) {
//     // Build user list
//     std::string names;
//     for (int memberFd : channel.getClients()) {
//         names += server->_clients[memberFd]->nickname + " ";
//     }
//     // 353 <requestingNick> = #channel :names
//     // example: "353 Alisa = #test :Bob Alisa"
//     std::string rplNamReply = "353 " + server->_clients[fd]->nickname +
//                               " = " + channel.getName() + " :" + names +
//                               "\r\n";
//     send(fd, rplNamReply.c_str(), rplNamReply.size(), 0);

//     // 366 <requestingNick> #channel :End of /NAMES list
//     std::string rplEndOfNames = "366 " + server->_clients[fd]->nickname +
//                                 " " + channel.getName() +
//                                 " :End of /NAMES list\r\n";
//     send(fd, rplEndOfNames.c_str(), rplEndOfNames.size(), 0);
// }

// /**
//  * Handle JOIN:
//  *   1) Check registration
//  *   2) Validate parameters
//  *   3) get/create channel
//  *   4) check +i, +l, etc.
//  *   5) check +k if channel has a key
//  *   6) add user
//  *   7) notify members
//  *   8) send name reply
//  */
// void handleJoinCommand(Server* server, int fd,
//                        const std::vector<std::string>& tokens,
//                        const std::string& rawCommand)
// {
//     (void)rawCommand; // not used, but might parse trailing for real IRC

//     // 1) Check if registered
//     if (!isClientRegistered(server, fd))
//         return;

//     // 2) "JOIN <channel> [<key>]"
//     if (!isJoinCommandValid(fd, tokens))
//         return;

//     std::string channelName = tokens[1];

//     // If user typed: JOIN #channel key
//     // then tokens.size() could be >= 3, so tokens[2] is the key
//     std::string userKey;
//     if (tokens.size() >= 3) {
//         userKey = tokens[2];
//     }

//     try {
//         // 3) find or create
//         Channel& channel = getOrCreateChannel(server, channelName);

//         // 4) check +i, +l
//         if (!basicChannelChecks(fd, channel)) {
//             return;
//         }

//         // 5) check +k
//         if (channel.hasMode('k')) {
//             // compare userKey with channel.getChannelKey()
//             if (userKey != channel.getChannelKey()) {
//                 // wrong key or no key
//                 std::string err = "475 " + channelName + " :Cannot join
//                 channel (+k)\r\n"; sendErrorMessage(fd, err); return;
//             }
//         }

//         // 6) add the user
//         channel.addClient(fd);

//         // 7) notify everyone
//         notifyChannelMembers(server, fd, channel);

//         // 8) send 353/366 to the new user
//         sendNameReply(fd, channel, server);

//     } catch (const std::exception& e) {
//         std::cerr << "Error handling JOIN: " << e.what() << std::endl;
//     }
// }
