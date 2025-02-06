#include "Kick.hpp"

#include <string>

#include "../include/Server.hpp"

/**
 * @brief Handles the KICK command from a client.
 *
 * This function processes the KICK command, which is used to remove a target
 * user from a specified channel. It first verifies that the sender is fully
 * registered, then checks whether enough parameters were provided. Next, it
 * attempts to locate the specified channel and finds the target user by
 * nickname. If the target user is found as a member of the channel, they are
 * removed. A KICK notification is then broadcast to all remaining channel
 * members. If the target user is not found, an error message is sent back to
 * the sender.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the KICK command.
 * @param tokens A vector of strings representing the tokenized command
 * (expected: "KICK", <channelName>, <targetNick>).
 * @param command The complete command string (unused in this implementation).
 */
void handleKickCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    // Check if the client (sender) is fully registered.
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Verify that enough parameters are provided: KICK <channel> <targetNick>
    if (tokens.size() < 3)
    {
        std::string reply = "461 KICK :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string channelName = tokens[1];
    std::string targetNick  = tokens[2];

    // Find the specified channel.
    auto it = server->getChannels().find(channelName);
    if (it == server->getChannels().end())
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    bool found = false;
    // Search for the target user in the channel.
    for (int cli_fd : it->second.getClients())
    {
        auto clientIt = server->getClients().find(cli_fd);
        if (clientIt == server->getClients().end())
        {
            continue;  // Skip if the client is no longer there
        }

        if (clientIt->second->getNickname() == targetNick)
        {
            it->second.removeClient(cli_fd);
            if (it->second.getClients().empty())
            {
                server->getChannels().erase(
                    it);  // Delete the channel if it is empty
            }

            std::string kickMsg = ":" + server->getClients()[fd]->getNickname() +
                                  " KICK " + channelName + " " + targetNick +
                                  " :Kicked\r\n";
            server->broadcastMessage(kickMsg, fd);
            found = true;
            break;
        }
    }
    // If no such target user is found, send an error message.
    if (!found)
    {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
    }
}

// /**
//  * @brief Handles the KICK command from a client.
//  *
//  * The KICK command is used to remove a user from a channel.
//  * The command format: `KICK <channel> <targetNick> [:<comment>]`
//  *
//  * Steps:
//  * 1. Check if the sender is fully registered.
//  * 2. Validate command parameters.
//  * 3. Locate the target channel.
//  * 4. Verify that the sender is a channel operator.
//  * 5. Find the target user and remove them from the channel.
//  * 6. Send a `KICK` message to all channel members.
//  * 7. If the channel is empty after the kick, delete it.
//  *
//  * @param server Pointer to the Server object managing the IRC server.
//  * @param fd The file descriptor of the client issuing the KICK command.
//  * @param tokens A vector of strings representing the parsed command.
//  * @param rawCommand The full raw command string.
//  */
// void handleKickCommand(Server* server, int fd,
//                        const std::vector<std::string>& tokens,
//                        const std::string&              rawCommand)
// {
//     //  Check if the sender is fully registered
//     if (server->_clients[fd]->authState != AUTH_REGISTERED)
//     {
//         std::string reply = "451 :You have not registered\r\n";
//         send(fd, reply.c_str(), reply.size(), 0);
//         return;
//     }

//     // Validate parameters (KICK <channel> <targetNick> [:<comment>])
//     if (tokens.size() < 3)
//     {
//         std::string reply = "461 KICK :Not enough parameters\r\n";
//         send(fd, reply.c_str(), reply.size(), 0);
//         return;
//     }

//     std::string channelName = tokens[1];
//     std::string targetNick  = tokens[2];

//     //  Parse optional comment (IRC trailing)
//     std::string comment = "Kicked";  // Default value
//     size_t      pos     = rawCommand.find(':');
//     if (pos != std::string::npos)
//     {
//         comment = rawCommand.substr(pos + 1);  // Extract everything after
//         ':'
//     }

//     //  Find the specified channel
//     auto it = server->_channels.find(channelName);
//     if (it == server->_channels.end())
//     {
//         std::string reply = "403 " + channelName + " :No such channel\r\n";
//         send(fd, reply.c_str(), reply.size(), 0);
//         return;
//     }

//     Channel& channel = it->second;

//     //  Check if the sender is a channel operator
//     if (!channel.isOperator(fd))
//     {
//         std::string reply =
//             "482 " + channelName + " :You're not a channel operator\r\n";
//         send(fd, reply.c_str(), reply.size(), 0);
//         return;
//     }

//     //  Find the target user (optimized search)
//     auto clientIt =
//         std::find_if(channel.getClients().begin(),
//         channel.getClients().end(),
//                      [&server, &targetNick](int cli_fd)
//                      {
//                          auto it = server->_clients.find(cli_fd);
//                          return it != server->_clients.end() &&
//                                 it->second->nickname == targetNick;
//                      });

//     if (clientIt == channel.getClients().end())
//     {
//         std::string reply = "401 " + targetNick + " :No such
//         nick/channel\r\n"; send(fd, reply.c_str(), reply.size(), 0); return;
//     }

//     int targetFd = *clientIt;

//     //  Construct and send KICK message to all channel members
//     std::string kickMsg = ":" + server->_clients[fd]->nickname + " KICK " +
//                           channelName + " " + targetNick + " :" + comment +
//                           "\r\n";

//     for (int member_fd : channel.getClients())
//     {
//         send(member_fd, kickMsg.c_str(), kickMsg.size(), 0);
//     }

//     // Send a QUIT message to the kicked user before disconnecting
//     std::string quitMsg = "ERROR :You were kicked from " + channelName +
//     "\r\n"; send(targetFd, quitMsg.c_str(), quitMsg.size(), 0);

//     //  Remove the user from the channel
//     channel.removeClient(targetFd);
//     channel.removeOperator(targetFd);

//     //  Delete the channel if it becomes empty
//     if (channel.getClients().empty())
//     {
//         server->_channels.erase(it);
//     }
// }
