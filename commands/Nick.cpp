#include <algorithm> 
#include <set>
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
    // Construct the NICK change message in the IRC protocol format.
    std::string message =
        ":" + oldNick + "!" + client->getUsername() + "@" + client->getHost() +
        " NICK :" + newNick + "\r\n";

    int fd = client->getFd();
    
    // Use a set to avoid duplicate sends (e.g., if a user is in multiple channels).
    std::set<int> notifiedClients;

    // Iterate through all channels on the server.
    for (std::map<std::string, Channel>::iterator it = server->getChannels().begin();
         it != server->getChannels().end(); ++it)
    {
        Channel& chan = it->second;

        // Check if the client changing the nickname is in this channel.
        if (chan.hasClient(fd))
        {
            // Notify all clients in the channel about the nickname change.
            for (int otherFd : chan.getClients())
            {
                // Ensure each client receives the message only once.
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
    // Check if a nickname argument was provided.
    if (tokens.size() < 2)
    {
        std::string reply = "431 :No nickname given\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string newNick = tokens[1];

    // Ensure the nickname is unique by checking all connected clients.
    for (const auto& pair : server->getClients())
    {
        if (pair.first != fd && pair.second->getNickname() == newNick)
        {
            std::string reply = "433 * " + newNick + " :Nickname is already in use\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }

    // Retrieve the client object associated with the given file descriptor.
    Client* client = server->getClients()[fd].get();
    if (!client) return;

    std::string oldNick = client->getNickname();

    // Set the new nickname.
    client->setNickname(newNick);

    // Reference to the client's authentication state.
    AuthState& st = client->authState;

    ///----------------------------------------
    // Handle different authentication states:
    ///----------------------------------------

    // If the user is not yet registered, update state and return.
    if (st == NOT_REGISTERED)
    {
        st = WAITING_FOR_USER;
        return;
    }

    // If the user was waiting for a nickname, update state accordingly.
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

    // If the user was waiting for a username, finalize registration if possible.
    if (st == WAITING_FOR_USER)
    {
        if (!client->getUsername().empty())
        {
            st = AUTH_REGISTERED;
            sendWelcome(server, fd);
        }
        return;
    }

    // If the user is already registered and changed their nickname, notify others.
    if (st == AUTH_REGISTERED && !oldNick.empty() && oldNick != newNick)
    {
        broadcastNickChange(server, client, oldNick, newNick);
    }
}
