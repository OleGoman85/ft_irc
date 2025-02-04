#include "Invite.hpp"

#include <string>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

void handleInviteCommand(Server* server, int fd,
                         const std::vector<std::string>& tokens,
                         const std::string& /*command*/)
{
    // 1️⃣ Проверка, что отправитель зарегистрирован
    if (server->_clients[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 2️⃣ Проверка достаточного количества аргументов
    if (tokens.size() < 3)
    {
        std::string reply = "461 INVITE :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string targetNick  = tokens[1];
    std::string channelName = tokens[2];

    // 3️⃣ Проверяем, существует ли канал
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end())
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    Channel& channel = it->second;

    // 4️⃣ Проверяем, состоит ли отправитель в канале
    if (!channel.hasClient(fd))
    {
        std::string reply =
            "442 " + channelName + " :You're not on that channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 5️⃣ Проверяем, оператор ли отправитель (если канал invite-only)
    if (channel.isInviteOnly() && !channel.isOperator(fd))
    {
        std::string reply =
            "482 " + channelName + " :You're not a channel operator\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 6️⃣ Проверяем, существует ли целевой пользователь
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

    // 7️⃣ Проверяем, не находится ли уже приглашённый в канале
    if (channel.hasClient(targetFd))
    {
        std::string reply = "443 " + targetNick + " " + channelName +
                            " :is already on channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 8️⃣ Добавляем в список приглашённых
    channel.inviteClient(targetFd);

    // 9️⃣ Отправляем подтверждение отправителю
    std::string senderNick = server->_clients[fd]->nickname;
    std::string reply =
        "341 " + senderNick + " " + targetNick + " " + channelName + "\r\n";
    send(fd, reply.c_str(), reply.size(), 0);

    // 🔟 Отправляем приглашение пользователю
    std::string inviteMsg = ":" + senderNick + " INVITE " + targetNick + " :" +
                            channelName + "\r\n";
    send(targetFd, inviteMsg.c_str(), inviteMsg.size(), 0);
}
