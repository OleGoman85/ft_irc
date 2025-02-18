#include "Who.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include <sstream>
#include <string>
#include <sys/socket.h>

/**
 * @brief Handles the WHO command from a client.
 *
 * The WHO command is used to retrieve a list of users based on a given target.
 * The target can either be:
 * 1. A channel (e.g., "#channel") - Lists all users in the channel.
 * 2. Omitted or "*" - Lists all users connected to the server.
 *
 * The function follows these steps:
 * - **Extract the target** from the command tokens (if provided).
 * - **If a channel is specified**:
 *   - Check if the channel exists. If not, send an error (403).
 *   - Iterate over all clients in the channel and send WHO information.
 * - **If no target or "*" is provided**:
 *   - Iterate over all connected clients and send WHO information.
 * - **Send an "End of WHO list" (315) message** to indicate completion.
 *
 * @param server Pointer to the Server instance.
 * @param fd The file descriptor of the client issuing the WHO command.
 * @param tokens Vector of strings containing the parsed command arguments.
 *               Expected format: `"WHO" [<target>]`
 * @param command The full command string (unused).
 */
void handleWhoCommand(Server* server, int fd, 
                      const std::vector<std::string>& tokens, 
                      const std::string& command) 
{
    (void)command; // Unused parameter

    std::string reply;
    std::string target;

    // Step 1: Extract target from tokens if provided
    if (tokens.size() >= 2) {
        target = tokens[1];
    }

    // Step 2: Handle WHO for a channel target (e.g., "#channel")
    if (!target.empty() && target[0] == '#') 
    {
        // Check if the channel exists
        auto it = server->getChannels().find(target);
        if (it == server->getChannels().end()) 
        {
            // Send error: No such channel (403)
            reply = "403 " + target + " :No such channel\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }

        // Iterate through all clients in the channel
        for (int client_fd : it->second.getClients()) 
        {
            Client* client = server->getClients()[client_fd].get();
            std::ostringstream oss;

            // Format WHO response (352)
            oss << "352 " << server->getClients()[fd]->getNickname() << " " 
                << target << " " << client->getUsername() << " " 
                << client->getHost() << " " << server->getServerName() << " " 
                << client->getNickname() << " H :0 " << client->getUsername() << "\r\n";

            reply = oss.str();
            send(fd, reply.c_str(), reply.size(), 0);
        }
    }
    else 
    {
        // Step 3: Handle WHO for all connected users (* or no target)
        for (const auto& pair : server->getClients()) 
        {
            Client* client = pair.second.get();
            std::ostringstream oss;

            // Format WHO response (352) for all users
            oss << "352 " << server->getClients()[fd]->getNickname() << " * " 
                << client->getUsername() << " " << client->getHost() << " " 
                << server->getServerName() << " " << client->getNickname() 
                << " H :0 " << client->getUsername() << "\r\n";

            reply = oss.str();
            send(fd, reply.c_str(), reply.size(), 0);
        }
    }

    // Step 4: Send End of WHO list message (315)
    reply = "315 " + server->getClients()[fd]->getNickname() + " :End of WHO list\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}
