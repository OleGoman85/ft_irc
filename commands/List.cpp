#include "List.hpp"
#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include <sstream>
#include <string>
#include <sys/socket.h>

/**
 * @brief Handles the LIST command from a client.
 *
 * The LIST command shows all existing channels on the server,
 * including the number of users in each and the channel's topic (if any).
 * 
 * According to IRC protocol:
 *  - "322" is RPL_LIST: <channel> <#visible> :<topic>
 *  - "323" is RPL_LISTEND: signals the end of the list.
 *
 * @param server   Pointer to the Server instance.
 * @param fd       File descriptor of the requesting client.
 * @param tokens   Tokenized command arguments (unused here).
 * @param command  The raw command string (unused here).
 */
void handleListCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& command)
{
    // We don't need 'tokens' or 'command' here, but we keep them to match signature.
    (void)tokens;
    (void)command;

    // For each channel on the server, we build a "322" response.
    //  "322 <nick> <channelName> <clientCount> :<topic>"
    // Then we send a final "323 <nick> :End of LIST" to mark completion.
    std::string reply;
    for (auto& pair : server->getChannels())
    {
        Channel& channel = pair.second;

        // Gather all the info in an IRC-compliant line.
        // <clientCount> is channel.getClients().size(), i.e. number of users in the channel.
        std::ostringstream oss;
        oss << "322 " << server->getClients()[fd]->getNickname() << " "
            << channel.getName() << " " << channel.getClients().size()
            << " :" << channel.getTopic() << "\r\n";
        
        // Convert stream to string and send to the requesting client.
        reply = oss.str();
        send(fd, reply.c_str(), reply.size(), 0);
    }

    // Finally, send "323", which is RPL_LISTEND: signals no more channels to list.
    reply = "323 " + server->getClients()[fd]->getNickname() + " :End of LIST\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}
