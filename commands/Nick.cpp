/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 11:30:54 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/05 12:54:17 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Nick.hpp"
#include "../include/Server.hpp"
#include <string>

/**
 * @brief Handles the NICK command from a client.
 *
 * This function processes the NICK command, which is used by a client to set or change
 * its nickname. The function first verifies that a nickname was provided. It then checks
 * if the desired nickname is already in use by another client. If the nickname is available,
 * the client's nickname is updated. Additionally, if the client's authentication state
 * indicates that they are waiting for the USER command, their state is updated accordingly.
 * A confirmation message is then sent to the client.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the NICK command.
 * @param tokens A vector of strings representing the tokenized command arguments (expected: "NICK" followed by <newNick>).
 * @param command The complete command string received from the client (unused in this implementation).
 */
void handleNickCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& /*command*/) 
{
    // Check if a new nickname was provided.
    if (tokens.size() < 2) {
        std::string reply = "431 :No nickname given\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string newNick = tokens[1];

    // Check if the nickname is already in use by another client.
    for (const auto& pair : server->getClients()) 
    {
        if (pair.second->getNickname() == newNick) {
            std::string reply = "433 * " + newNick + " :Nickname is already in use\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }

    // Set the new nickname for the client.
    server->getClients()[fd]->setNickname(newNick);

    // Update the client's authentication state.
    // If the client was waiting for the USER command, mark them as fully registered.
    // Otherwise, if the client was just waiting for a nickname, update the state to waiting for USER.
    if (server->getClients()[fd]->authState == WAITING_FOR_USER)
        server->getClients()[fd]->authState = AUTH_REGISTERED;
    else if (server->getClients()[fd]->authState == WAITING_FOR_NICK)
        server->getClients()[fd]->authState = WAITING_FOR_USER;
    
    // Send a confirmation response to the client indicating successful nickname setting.
    std::string reply = "001 " + server->getClients()[fd]->getNickname() + " :Nickname set successfully\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}


/*
Data Buffer:
reply.c_str() is a pointer to the start of the string reply.

reply.size() is the length of the string, specifying how many bytes from the buffer should be sent.

Data Transmission:
The system call send sends data from the specified buffer (reply.c_str()) through the socket identified by the file descriptor fd.

Return Value:
send returns the number of bytes successfully sent to the client.
If an error occurs, it returns -1, and additional error information can be retrieved using errno.

*/