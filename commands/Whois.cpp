#include "Whois.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include <sstream>
#include <string>
#include <sys/socket.h>

/**
 * @brief Handles the WHOIS command from a client.
 *
 * The WHOIS command provides information about a specific user on the server.
 * The response includes:
 *  - Nickname, username, host, and real name of the target user.
 *  - An error if the target user does not exist.
 *  - A final "End of WHOIS" message to indicate completion.
 *
 * Steps:
 * - **Check for valid parameters**: Ensure the command includes a nickname.
 * - **Find the target user**: Search for the user by nickname in the server's client list.
 * - **If the user is not found**, send error 401 ("No such nick/channel").
 * - **Format and send WHOIS information (311)**: Includes nickname, username, host, and real name.
 * - **Send WHOIS completion message (318)**: Indicates the end of the WHOIS response.
 *
 * Numeric Replies Used:
 *  - 461 WHOIS :Not enough parameters (Missing nickname)
 *  - 401 <nickname> :No such nick/channel (User not found)
 *  - 311 <requester> <nickname> <username> <host> * :<real name> (User info)
 *  - 318 <requester> <nickname> :End of WHOIS (Completion)
 *
 * @param server Pointer to the Server instance.
 * @param fd The file descriptor of the client requesting WHOIS.
 * @param tokens Vector containing the parsed command arguments.
 *               Expected format: "WHOIS <nickname>"
 * @param command The full command string (unused).
 */
void handleWhoisCommand(Server* server, int fd, 
                        const std::vector<std::string>& tokens, 
                        const std::string& command) 
{
    (void)command; // Unused parameter

    // Step 1: Validate parameters (Check if a nickname is provided)
    if (tokens.size() < 2) 
    {
        std::string reply = "461 WHOIS :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Step 2: Extract the target nickname
    std::string targetNick = tokens[1];
    Client* targetClient = nullptr;

    // Step 3: Search for the target user in the server's client list
    for (auto& pair : server->getClients()) 
    {
        if (pair.second->getNickname() == targetNick) 
        {
            targetClient = pair.second.get();
            break;
        }
    }

    // Step 4: Handle case where target user is not found
    if (!targetClient) 
    {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Step 5: Prepare WHOIS response (311) containing user details
    std::ostringstream oss;
    std::string realName = targetClient->getRealName();
    if (realName.empty()) 
    {
        realName = "Real name not set"; // Fallback if real name is not provided
    }

    // Format WHOIS user information response (311)
    oss << "311 " << server->getClients()[fd]->getNickname() << " " 
        << targetNick << " " << targetClient->getUsername() << " " 
        << targetClient->getHost() << " * :" << realName << "\r\n";

    std::string reply = oss.str();
    send(fd, reply.c_str(), reply.size(), 0);

    // Step 6: Send WHOIS completion message (318)
    reply = "318 " + server->getClients()[fd]->getNickname() + " " 
            + targetNick + " :End of WHOIS\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}
