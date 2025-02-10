#include "Kick.hpp"

#include <sstream>
#include <string>

#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"

/**
 * @brief Finds a client FD by nickname.
 *
 * @param server Pointer to the Server.
 * @param nickname The nickname to search for.
 * @return The file descriptor of the found client, or -1 if not found.
 */
static int findUserFdByNick(Server* server, const std::string& nickname)
{
    for (std::map<int, std::unique_ptr<Client> >::iterator it =
             server->getClients().begin();
         it != server->getClients().end(); ++it)
    {
        if (it->second->getNickname() == nickname) return it->first;
    }
    return -1;
}

/**
 * @brief Checks if a given client is on the specified channel.
 *
 * @param server Pointer to the Server.
 * @param fd The file descriptor of the client to check.
 * @param channelName The name of the channel.
 * @return true if the client is in the channel, false otherwise.
 */
static bool isUserInChannel(Server* server, int fd,
                            const std::string& channelName)
{
    if (server->getChannels().find(channelName) == server->getChannels().end())
        return false;

    Channel& chan = server->getChannels()[channelName];
    return chan.hasClient(fd);
}

/**
 * @brief Checks if a given client is an operator in the specified channel.
 *
 * @param server Pointer to the Server.
 * @param fd The file descriptor of the client to check.
 * @param channelName The name of the channel.
 * @return true if the client is an operator on the channel, false otherwise.
 */
static bool isUserOperatorInChannel(Server* server, int fd,
                                    const std::string& channelName)
{
    if (server->getChannels().find(channelName) == server->getChannels().end())
        return false;

    Channel& chan = server->getChannels()[channelName];
    return chan.isOperator(fd);
}

/**
 * @brief Counts how many operators are in the channel.
 *
 * @param chan Reference to the Channel object.
 * @return The count of operator FDs in the channel.
 */
static int countOperators(const Channel& chan)
{
    int count = 0;
    for (int cliFd : chan.getClients())
    {
        if (chan.isOperator(cliFd)) count++;
    }
    return count;
}

/**
 * @brief Handles the KICK command from a client.
 *
 * removes a target user from the specified channel.
 * The overall logic is:
 *   Check registration and parameters.
 *   Verify that the channel exists, and that the kicker is on it + is an
 * operator.
 *   Find the target user by nickname; check if they're on the channel.
 *   If the target user is the last operator in a channel that still has
 * other participants, deny the KICK (analogous to the PART logic).
 *   Otherwise, broadcast a KICK message and remove the target from the
 * channel.
 *   If the channel becomes empty, delete it.
 *
 * Numeric replies used:
 *   - 451 :You have not registered
 *   - 461 KICK :Not enough parameters
 *   - 403 <channel> :No such channel
 *   - 442 <channel> :You're not on that channel
 *   - 482 <channel> :You're not channel operator
 *   - 401 <nickname> :No such nick
 *   - 441 <nickname> <channel> :They aren't on that channel
 *   - 482 <channel> :Cannot remove last operator (reused numeric)
 */
void handleKickCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    if (tokens.size() < 3)
    {
        std::string reply = "461 KICK :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    std::string channelName = tokens[1];
    std::string targetNick  = tokens[2];

    std::map<std::string, Channel>& chanMap = server->getChannels();
    if (chanMap.find(channelName) == chanMap.end())
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    Channel& channelObj = chanMap[channelName];

    if (!isUserInChannel(server, fd, channelName))
    {
        std::string reply =
            "442 " + channelName + " :You're not on that channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (!isUserOperatorInChannel(server, fd, channelName))
    {
        std::string reply =
            "482 " + channelName + " :You're not channel operator\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    int targetFd = findUserFdByNick(server, targetNick);
    if (targetFd == -1)
    {
        std::string reply = "401 " + targetNick + " :No such nick\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (!isUserInChannel(server, targetFd, channelName))
    {
        std::string reply = "441 " + targetNick + " " + channelName +
                            " :They aren't on that channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (channelObj.isOperator(targetFd))
    {
        size_t totalUsers = channelObj.getClients().size();
        if (totalUsers > 1)
        {
            int opCount = countOperators(channelObj);
            if (opCount == 1)
            {
                std::string reply =
                    "482 " + channelName + " :Cannot remove last operator\r\n";
                send(fd, reply.c_str(), reply.size(), 0);
                return;
            }
        }
    }

    std::string comment;
    if (tokens.size() > 3)
    {
        std::ostringstream oss;
        for (size_t i = 3; i < tokens.size(); ++i)
        {
            if (i > 3) oss << " ";
            oss << tokens[i];
        }
        comment = oss.str();
    }
    else
    {
        comment = server->getClients()[fd]->getNickname();
    }

    channelObj.removeClient(targetFd);

    if (channelObj.getClients().empty())
    {
        chanMap.erase(channelName);
    }

    // Classic format: :<kickerNick>!<user>@<host> KICK <channel> <targetNick>
    // :<comment>
    std::string kickMsg = ":" + server->getClients()[fd]->getNickname() +
                          "!user@localhost KICK " + channelName + " " +
                          targetNick + " :" + comment + "\r\n";
    for (int memFd : channelObj.getClients())
    {
        if (memFd != fd) send(memFd, kickMsg.c_str(), kickMsg.size(), 0);
    }
    send(targetFd, kickMsg.c_str(), kickMsg.size(), 0);
    send(fd, kickMsg.c_str(), kickMsg.size(), 0);
}
