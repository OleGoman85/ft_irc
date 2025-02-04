#include "Invite.hpp"

#include <string>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

void handleInviteCommand(Server* server, int fd,
                         const std::vector<std::string>& tokens,
                         const std::string& /*command*/)
{
    if (server->_clients[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (tokens.size() < 3)
    {
        std::string reply = "461 INVITE :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string targetNick  = tokens[1];
    std::string channelName = tokens[2];

    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end())
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    Channel& channel = it->second;

    if (!channel.hasClient(fd))
    {
        std::string reply =
            "442 " + channelName + " :You're not on that channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (channel.isInviteOnly() && !channel.isOperator(fd))
    {
        std::string reply =
            "482 " + channelName + " :You're not a channel operator\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    int targetFd = -1;
    for (const auto& pair : server->_clients)
    {
        if (pair.second->nickname == targetNick)
        {
            targetFd = pair.first;
            break;
        }
    }
    if (targetFd == -1)
    {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (channel.hasClient(targetFd))
    {
        std::string reply = "443 " + targetNick + " " + channelName +
                            " :is already on channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    channel.inviteClient(targetFd);

    std::string senderNick = server->_clients[fd]->nickname;
    std::string reply =
        "341 " + senderNick + " " + targetNick + " " + channelName + "\r\n";
    send(fd, reply.c_str(), reply.size(), 0);

    std::string inviteMsg = ":" + senderNick + " INVITE " + targetNick + " :" +
                            channelName + "\r\n";
    send(targetFd, inviteMsg.c_str(), inviteMsg.size(), 0);
}
