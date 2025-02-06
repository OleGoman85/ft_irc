

#include "Invite.hpp"

#include <string>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

/**
 * @brief Finds a user by getNickname() and a channel by name.
 * @return A pair: if the user is found, targetFd is set to their file
 * descriptor (otherwise -1); if the channel is found, a pointer to it is
 * returned, otherwise nullptr.
 */
std::pair<int, Channel*> findUserAndChannel(Server*            server,
                                            const std::string& targetNick,
                                            const std::string& channelName)
{
    int targetFd = -1;
    for (const auto& pair : server->getClients())
    {
        if (pair.second->getNickname() == targetNick)
        {
            targetFd = pair.first;
            break;
        }
    }

    auto     it = server->getChannels().find(channelName);
    Channel* channel =
        (it != server->getChannels().end()) ? &it->second : nullptr;

    return {targetFd, channel};
}

/**
 * @brief Checks if a user (fd) can invite others to a channel.
 * If not, sends the appropriate error message.
 */
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

/**
 * @brief Processes the invite:
 *  - If the target user is already in the channel (including if the inviter
 * tries to invite themselves), sends error 443.
 *  - Otherwise, adds the user to the invite list and sends notifications to
 * both parties.
 */
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

    std::string inviteMsg = ":" + server->getClients()[fd]->getNickname() +
                            " INVITE " + targetNick + " " + channelName +
                            "\r\n";
    send(targetFd, inviteMsg.c_str(), inviteMsg.size(), 0);

    std::string confirmMsg = "341 " + server->getClients()[fd]->getNickname() +
                             " " + targetNick + " " + channelName + "\r\n";
    send(fd, confirmMsg.c_str(), confirmMsg.size(), 0);
}

/**
 * @brief Handles the INVITE command.
 * Steps performed:
 *  1. Checks if the client is registered (otherwise 451).
 *  2. Checks if the required parameters are provided (otherwise 461).
 *  3. Searches for the channel and target user (errors 403 and 401).
 *  4. Verifies if the inviter has the necessary rights (errors 442/482).
 *  5. If the target user is already in the channel (including self-invite) —
 * error 443.
 *  6. Otherwise, processes the invite.
 */
void handleInviteCommand(Server* server, int fd,
                         const std::vector<std::string>& tokens,
                         const std::string& /*command*/)
{
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        std::string err = "451 :You have not registered\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    if (tokens.size() < 3)
    {
        std::string err = "461 INVITE :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    std::string targetNick  = tokens[1];
    std::string channelName = tokens[2];

    auto [targetFd, channel] =
        findUserAndChannel(server, targetNick, channelName);

    bool hasErrors = false;
    if (!channel)
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        hasErrors = true;
    }
    if (targetFd == -1)
    {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        hasErrors = true;
    }
    if (hasErrors) return;

    if (!canUserInvite(fd, channel, channelName)) return;

    processInvite(server, fd, targetFd, channel, targetNick, channelName);
}