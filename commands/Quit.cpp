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
 * - Building a quit message (combining all tokens after the command).
 * - Sending the QUIT message to all clients sharing channels with the quitting
 * client.
 * - Removing the client from all channels (and erasing empty channels).
 * - Sending the QUIT message to the client.
 * - Removing the client from the server.
 *
 * @param server Pointer to the Server instance.
 * @param fd The file descriptor of the quitting client.
 * @param tokens The parsed command tokens (e.g. "QUIT", "reason ...").
 * @param command The full command string (unused).
 */
void handleQuitCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    if (server->getClients().find(fd) == server->getClients().end()) return;

    Client*     client = server->getClients()[fd].get();
    std::string nick   = client->getNickname();
    std::string user =
        client->getUsername().empty() ? "unknown" : client->getUsername();
    std::string host = client->getHost();

    std::string prefix = ":" + nick + "!" + user + "@" + host;

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
        quitReason = "Client has quit";
    }

    std::string              quitMsg = prefix + " QUIT :" + quitReason + "\r\n";
    std::vector<std::string> channelsToRemove;

    for (auto& pair : server->getChannels())
    {
        Channel& chan = pair.second;
        if (chan.hasClient(fd))
        {
            for (int cli_fd : chan.getClients())
            {
                if (cli_fd != fd)
                {
                    send(cli_fd, quitMsg.c_str(), quitMsg.size(), 0);
                }
            }
            chan.removeClient(fd);
            if (chan.getClients().empty())
            {
                channelsToRemove.push_back(pair.first);
            }
        }
    }
    for (const std::string& chanName : channelsToRemove)
    {
        server->getChannels().erase(chanName);
    }
    send(fd, quitMsg.c_str(), quitMsg.size(), 0);
    server->removeClient(fd);
}
