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
                       const std::string& command)
{
    (void)command;
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    if (tokens.size() < 2)
    {
        std::string reply = "461 JOIN :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    std::string channelName = tokens[1];
    if (channelName.empty() || channelName[0] != '#')
    {
        std::string reply = "479 " + channelName + " :Illegal channel name. Channel names must start with '#'\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    auto it = server->getChannels().find(channelName);
    if (it != server->getChannels().end() && it->second.hasClient(fd))
    {
        std::string reply = "443 " + channelName + " :You are already in the channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    bool isFirstUser = (it == server->getChannels().end());
    if (isFirstUser)
    {
        auto emplaceResult = server->getChannels().emplace(channelName, Channel(channelName));
        if (!emplaceResult.second)
        {
            std::cerr << "Failed to create channel: " << channelName << std::endl;
            return;
        }
        it = emplaceResult.first;
    }
    if (it->second.isInviteOnly() && !it->second.isOperator(fd) && !it->second.isInvited(fd))
    {
        std::string reply = "473 " + channelName + " :Cannot join channel (+i mode set)\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    if (it->second.getUserLimit() > 0 && static_cast<int>(it->second.getClients().size()) >= it->second.getUserLimit())
    {
        std::string reply = "471 " + channelName + " :Channel is full\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    if (it->second.hasMode('k'))
    {
        if (tokens.size() < 3 || it->second.getChannelKey() != tokens[2])
        {
            std::string reply = "475 " + channelName + " :Cannot join channel (+k mode set)\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }
    it->second.addClient(fd);
    if (it->second.isInvited(fd))
    {
        it->second.removeInvite(fd);
    }
    if (isFirstUser)
    {
        it->second.addOperator(fd);
        std::string opMsg = ":" + server->getClients()[fd]->getNickname() + " MODE " + channelName + " +o " + server->getClients()[fd]->getNickname() + "\r\n";
        send(fd, opMsg.c_str(), opMsg.size(), 0);
    }
    std::string welcomeMsg = "NOTICE " + channelName + " :Welcome to " + channelName;
    if (isFirstUser)
    {
        welcomeMsg += "! You are the first user and the operator.";
    }
    welcomeMsg += "\r\n";
    send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    std::string userList = "353 " + server->getClients()[fd]->getNickname() + " = " + channelName + " :";
    for (int cli_fd : it->second.getClients())
    {
        if (it->second.isOperator(cli_fd))
        {
            userList += "@";
        }
        userList += server->getClients()[cli_fd]->getNickname() + " ";
    }
    userList += "\r\n";
    send(fd, userList.c_str(), userList.size(), 0);
    Client* c = server->getClients()[fd].get();
    std::string nick = c->getNickname();
    std::string user = c->getUsername();
    if (user.empty())
        user = "unknown";
    std::string host = c->getHost();
    std::string prefix = ":" + nick + "!" + user + "@" + host;
    std::string joinMsg = prefix + " JOIN " + channelName + "\r\n";
    send(fd, joinMsg.c_str(), joinMsg.size(), 0);
    for (int cli_fd : it->second.getClients())
    {
        if (cli_fd != fd)
        {
            send(cli_fd, joinMsg.c_str(), joinMsg.size(), 0);
        }
    }
    if (!it->second.getTopic().empty())
    {
        std::string topicMsg = "332 " + server->getClients()[fd]->getNickname() + " " + channelName + " :" + it->second.getTopic() + "\r\n";
        send(fd, topicMsg.c_str(), topicMsg.size(), 0);
    }
}
