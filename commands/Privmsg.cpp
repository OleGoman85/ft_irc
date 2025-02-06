/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:31:16 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/05 13:03:07 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Privmsg.hpp"
#include "../include/Server.hpp"
#include <string>

/**
 * @brief Handles the PRIVMSG command from a client.
 *
 * This function processes the PRIVMSG command, which is used to send private messages either
 * to a specific user or to a channel. It first verifies that the client is fully registered and 
 * that enough parameters have been provided. 
 *
 * If the target starts with '#' (indicating a channel), it checks that the channel exists and that 
 * the sender is a member of the channel. If the sender is not on the channel, an error message is returned.
 *
 * The function then extracts the actual message content from the command (after the ':' delimiter) 
 * and sends the message:
 * - For channel messages, it broadcasts the message to all channel members except the sender.
 * - For private messages (target is a nickname), it sends the message directly to the specified user.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the PRIVMSG command.
 * @param tokens A vector of strings containing the tokenized command arguments.
 *               Expected format: "PRIVMSG", <target>, and the message text after a ':'.
 * @param command The complete command string received from the client.
 */
void handlePrivmsgCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) {
    // Check if the client is fully registered.
    if (server->getClients()[fd]->authState != AUTH_REGISTERED) 
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Check that enough parameters are provided.
    if (tokens.size() < 3) {
        std::string reply = "461 PRIVMSG :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Extract the target from the tokens.
    std::string target = tokens[1];
    
    // If the target is a channel (starts with '#'), ensure the sender is a member.
    if (!target.empty() && target[0] == '#') {
        auto channelIt = server->getChannels().find(target);
        if (channelIt == server->getChannels().end()) {
            std::string reply = "403 " + target + " :No such channel\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        // If the sender is not part of the channel, send an error.
        if (!channelIt->second.hasClient(fd)) {
            std::string reply = "442 " + target + " :You're not on that channel\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }
    
    // Determine the start position of the message text.
    size_t msgStart = command.find(':');
    if (msgStart == std::string::npos) {
        // If ':' is not found, assume the message starts after the target name plus a space.
        msgStart = command.find(target) + target.size() + 1;
    } else {
        // Otherwise, skip the ':' character.
        msgStart++;
    }
    
    // Extract the message from the command.
    std::string message = command.substr(msgStart);
    
    // If the target is a channel, broadcast the message to all members except the sender.
    if (!target.empty() && target[0] == '#') 
    {
        auto it = server->getChannels().find(target);
        if (it != server->getChannels().end()) {
            std::string fullMsg = ":" + server->getClients()[fd]->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
            for (int cli_fd : it->second.getClients()) {
                if (cli_fd != fd)
                    send(cli_fd, fullMsg.c_str(), fullMsg.size(), 0);
            }
        } else {
            std::string reply = "403 " + target + " :No such channel\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
        }
    }
    // Otherwise, treat the target as a getNickname() and send a private message.
    else {
        bool found = false;
        for (const auto& pair : server->getClients()) {
            if (pair.second->getNickname() == target) {
                std::string fullMsg = ":" + server->getClients()[fd]->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
                send(pair.first, fullMsg.c_str(), fullMsg.size(), 0);
                found = true;
                break;
            }
        }
        if (!found) {
            std::string reply = "401 " + target + " :No such nick/channel\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
        }
    }
}
