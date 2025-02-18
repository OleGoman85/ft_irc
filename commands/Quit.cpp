#include "Quit.hpp"
#include <sstream>
#include <string>
#include <vector>
#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"

/**
 * @brief Handles the QUIT command from a client.
 *
 * This function processes the QUIT command by:
 * - Constructing a quit message, including a reason if provided.
 * - Broadcasting the quit message to all clients in shared channels.
 * - Removing the client from all channels and erasing empty channels.
 * - Sending the quit message directly to the quitting client.
 * - Removing the client from the server.
 *
 * @param server Pointer to the Server instance.
 * @param fd The file descriptor of the quitting client.
 * @param tokens The parsed command tokens (e.g., "QUIT", "reason ...").
 * @param command The full command string (unused).
 */
void handleQuitCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    // Check if the client exists in the server's client list.
    if (server->getClients().find(fd) == server->getClients().end()) return;

    // Retrieve the quitting client's details.
    Client*     client = server->getClients()[fd].get();
    std::string nick   = client->getNickname();
    std::string user   = client->getUsername().empty() ? "unknown" : client->getUsername();
    std::string host   = client->getHost();

    // Construct the prefix for the quit message.
    std::string prefix = ":" + nick + "!" + user + "@" + host;

    // Construct the quit reason.
    std::string quitReason;
    if (tokens.size() > 1)
    {
        std::ostringstream oss;
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            if (i > 1) oss << " ";
            oss << tokens[i];
        }
        quitReason = oss.str();
    }
    else
    {
        quitReason = "Client has quit"; // Default quit message if no reason is provided.
    }

    // Construct the full QUIT message to be sent to all relevant clients.
    std::string quitMsg = prefix + " QUIT :" + quitReason + "\r\n";

    // List of channels that should be removed if they become empty.
    std::vector<std::string> channelsToRemove;

    // Iterate over all channels on the server to notify other clients and remove the quitter.
    for (auto& pair : server->getChannels())
    {
        Channel& chan = pair.second;

        // Check if the quitting client is a member of the channel.
        if (chan.hasClient(fd))
        {
            // Broadcast the QUIT message to all other clients in the channel.
            for (int cli_fd : chan.getClients())
            {
                if (cli_fd != fd) // Skip sending to the quitting client.
                {
                    send(cli_fd, quitMsg.c_str(), quitMsg.size(), 0);
                }
            }

            // Remove the quitting client from the channel.
            chan.removeClient(fd);

            // If the channel becomes empty after removal, mark it for deletion.
            if (chan.getClients().empty())
            {
                channelsToRemove.push_back(pair.first);
            }
        }
    }

    // Remove all empty channels from the server.
    for (const std::string& chanName : channelsToRemove)
    {
        server->getChannels().erase(chanName);
    }

    // Send the QUIT message directly to the quitting client.
    send(fd, quitMsg.c_str(), quitMsg.size(), 0);

    // Remove the client from the server's client list.
    server->removeClient(fd);
}
