#include "Invite.hpp"

#include <string>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

/**
 * @brief Returns (targetFd, channel*) without sending any errors.
 */
std::pair<int, Channel*> findUserAndChannel(Server*            server,
                                            const std::string& targetNick,
                                            const std::string& channelName)
{
    int targetFd = -1;
    for (const auto& pair : server->_clients)
    {
        if (pair.second->nickname == targetNick)
        {
            targetFd = pair.first;
            break;
        }
    }

    auto     it      = server->_channels.find(channelName);
    Channel* channel = (it != server->_channels.end()) ? &it->second : nullptr;

    return {targetFd, channel};
}

bool canUserInvite(int fd, Channel* channel, const std::string& channelName)
{
    if (!channel->hasClient(fd))
    {
        std::string reply =
            "442 " + channelName + " :You're not on that channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return false;
    }

    if (!channel->isOperator(fd))
    {
        std::string reply =
            "482 " + channelName + " :You're not a channel operator\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return false;
    }

    return true;
}

void processInvite(Server* server, int fd, int targetFd, Channel* channel,
                   const std::string& targetNick,
                   const std::string& channelName)
{
    if (channel->hasClient(targetFd))
    {
        std::string reply = "443 " + targetNick + " " + channelName +
                            " :is already on channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    channel->inviteClient(targetFd);

    std::string inviteMsg = ":" + server->_clients[fd]->nickname + " INVITE " +
                            targetNick + " " + channelName + "\r\n";
    send(targetFd, inviteMsg.c_str(), inviteMsg.size(), 0);

    std::string confirmMsg = "341 " + server->_clients[fd]->nickname + " " +
                             targetNick + " " + channelName + "\r\n";
    send(fd, confirmMsg.c_str(), confirmMsg.size(), 0);
}

/**
 * @brief Handles the INVITE command, sending *all* relevant errors if multiple
 * issues are found.
 */
void handleInviteCommand(Server* server, int fd,
                         const std::vector<std::string>& tokens,
                         const std::string& /*command*/)
{
    // 1) Check registration
    if (server->_clients[fd]->authState != AUTH_REGISTERED)
    {
        send(fd, "451 :You have not registered\r\n", 30, 0);
        return;
    }

    // 2) Check parameters
    if (tokens.size() < 3)
    {
        send(fd, "461 INVITE :Not enough parameters\r\n", 36, 0);
        return;
    }

    std::string targetNick  = tokens[1];
    std::string channelName = tokens[2];

    // 3) Find user & channel (no errors sent here)
    auto [targetFd, channel] =
        findUserAndChannel(server, targetNick, channelName);

    // We'll collect all errors in a flag
    bool hasErrors = false;

    // 4) Check if channel exists
    if (!channel)
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        hasErrors = true;
    }

    // 5) Check if user exists
    if (targetFd == -1)
    {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        hasErrors = true;
    }

    // 6) Check for self-invite
    if (server->_clients[fd]->nickname == targetNick)
    {
        send(fd, "481 :You cannot invite yourself\r\n", 33, 0);
        hasErrors = true;
    }

    // If we already have errors, stop here
    if (hasErrors) return;

    // 7) Now we can check operator privileges
    if (!canUserInvite(fd, channel, channelName)) return;

    // 8) Everything is ok, do invite
    processInvite(server, fd, targetFd, channel, targetNick, channelName);
}

// Успела протестировать в INVITE:
// ответы сверяла с протоколом RFC 2812 и результатами тестов на Libera.Chat 


// Позитивные тесты
// ✅ Обычное приглашение: оператор приглашает пользователя, который не в
// канале.
//    INVITE Alisa #cde
// ✅ Приглашение в invite-only канал: оператор приглашает пользователя в канал
// с +i.
//    INVITE Alisa #private
// ✅ Правильные ответы при успешном приглашении:
//    341 <inviter> <invitee> <channel> — подтверждение для пригласившего.
//    :<inviter> INVITE <invitee> <channel> — сообщение-приглашение для
//    приглашенного.

// Негативные тесты
// ❌ Попытка приглашения без регистрации
//    INVITE Alisa #cde → 451 :You have not registered.
// ❌ Попытка приглашения без параметров
//    INVITE → 461 INVITE :Not enough parameters.
// ❌ Попытка пригласить самого себя
//    INVITE Masha #cde → 481 :You cannot invite yourself.
// ❌ Попытка пригласить несуществующего пользователя
//    INVITE sdfsdf #cde → 401 sdfsdf :No such nick/channel.
// ❌ Попытка пригласить в несуществующий канал
//    INVITE Alisa #sdfs → 403 #sdfs :No such channel.
// ❌ Попытка пригласить в несуществующий канал и несуществующего пользователя
//    INVITE sdfsdf #sdfs
//    → 401 sdfsdf :No such nick/channel
//    → 403 #sdfs :No such channel.
// ❌ Попытка пригласить в канал без прав оператора
//    INVITE Alisa #cde (отправитель не оператор) → 482 #cde :You're not a
//    channel operator.
// ❌ Попытка пригласить пользователя, который уже в канале
//    INVITE Alisa #cde, если Alisa уже в канале → 443 Alisa #cde :is already on
//    channel.
// ❌ Проверка, что инвайт перестает действовать после захода в канал
//    INVITE Alisa #cde → JOIN Alisa #cde → PART Alisa #cde
//    Повторный JOIN без нового инвайта → 473 #cde :Cannot join channel (+i mode
//    set).
// ❌ Проверка, что оператор теряет +o при выходе
//    JOIN #cde → оператор выходит (PART/QUIT) → повторный JOIN → операторский
//    статус сбрасывается.
// ❌ Попытка пригласить, если пользователь не находится в канале
//    INVITE Alisa #cde, если приглашающий не в канале → 442 #cde :You're not on
//    that channel.
// ❌ Самоприглашение в несуществующий канал
//    INVITE Alisa #sdf 
//    → 403 #sdf :No such channel 
//    → 481 :You cannot invite yourself.



//!! проблема с приглашением несуществующего пользователя, не находясь в канале INVITE sdfsdf #cde
//ожидаемый ответ:  442 #cde :You're not on that channel
//                  401 sdfsdf :No such nick/channel
//наш ответ:    401 sdfsdf :No such nick/channel
