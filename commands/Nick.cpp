#include <algorithm>  // std::transform
#include <set>        // std::set

#include "../include/Replies.hpp"
#include "../include/Server.hpp"

/**
 * @brief Sends a nickname change notification to all clients in shared channels.
 *
 * This function ensures that a client receives the nickname change message only once,
 * even if they are in multiple channels.
 *
 * @param server Pointer to the Server instance.
 * @param client Pointer to the Client whose nickname changed.
 * @param oldNick The previous nickname.
 * @param newNick The new nickname.
 */
static void broadcastNickChange(Server* server, Client* client, 
                                const std::string& oldNick, 
                                const std::string& newNick)
{
    std::string message =
        ":" + oldNick + "!" + client->getUsername() + "@" + client->getHost() +
        " NICK :" + newNick + "\r\n";

    int fd = client->getFd();
    
    // Use a set to avoid duplicate sends (e.g., if a user is in multiple channels).
    std::set<int> notifiedClients;

    for (std::map<std::string, Channel>::iterator it = server->getChannels().begin();
         it != server->getChannels().end(); ++it)
    {
        Channel& chan = it->second;
        if (chan.hasClient(fd))
        {
            for (int otherFd : chan.getClients())
            {
                if (otherFd != fd && notifiedClients.insert(otherFd).second)
                {
                    send(otherFd, message.c_str(), message.size(), 0);
                }
            }
        }
    }

    // Send the nickname change message directly to the user
    send(fd, message.c_str(), message.size(), 0);
}

/**
 * @brief Handles the NICK command from a client.
 *
 * This function ensures that a nickname is unique, changes it if valid,
 * and notifies all relevant clients.
 *
 * @param server Pointer to the Server instance.
 * @param fd The client's file descriptor.
 * @param tokens The parsed command tokens.
 * @param command The full command string (unused).
 */
void handleNickCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    if (tokens.size() < 2)
    {
        std::string reply = "431 :No nickname given\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string newNick = tokens[1];

    for (const auto& pair : server->getClients())
    {
        if (pair.first != fd && pair.second->getNickname() == newNick)
        {
            std::string reply = "433 * " + newNick + " :Nickname is already in use\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }

    Client* client = server->getClients()[fd].get();
    if (!client) return;

    std::string oldNick = client->getNickname();

    client->setNickname(newNick);

    AuthState& st = client->authState;

    if (st == NOT_REGISTERED)
    {
        st = WAITING_FOR_USER;
        return;
    }

    if (st == WAITING_FOR_NICK)
    {
        st = WAITING_FOR_USER;
        if (!client->getUsername().empty())
        {
            st = AUTH_REGISTERED;
            sendWelcome(server, fd);
        }
        return;
    }

    if (st == WAITING_FOR_USER)
    {
        if (!client->getUsername().empty())
        {
            st = AUTH_REGISTERED;
            sendWelcome(server, fd);
        }
        return;
    }

    if (st == AUTH_REGISTERED && !oldNick.empty() && oldNick != newNick)
    {
        broadcastNickChange(server, client, oldNick, newNick);
    }
}
