/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alisa <alisa@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 11:30:54 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/31 10:19:06 by alisa            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Nick.hpp"

#include <iostream>
#include <string>

#include "../include/Server.hpp"

/**
 * @brief Handles the NICK command from a client.
 */
void handleNickCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    // Check if a new nickname was provided.
    if (tokens.size() < 2)
    {
        std::string reply = "431 :No nickname given\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string newNick = tokens[1];

    // Check if the nickname is already in use by another client.
    for (const auto& pair : server->getClients())  // проверяю getClients()
    {
        if (pair.second->nickname == newNick)
        {
            std::string reply =
                "433 * " + newNick + " :Nickname is already in use\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }

    // ещё проверяю
    auto& clients = server->getClients();
    if (clients.find(fd) == clients.end())
    {
        std::cerr << "Error: Client not found for fd " << fd << "\n";
        return;
    }

    // Set the new nickname for the client.
    clients[fd]->nickname = newNick;

    // Update the client's authentication state.
    if (clients[fd]->authState == WAITING_FOR_USER)
        clients[fd]->authState = AUTH_REGISTERED;
    else if (clients[fd]->authState == WAITING_FOR_NICK)
        clients[fd]->authState = WAITING_FOR_USER;

    // Send a confirmation response to the client.
    std::string reply = "001 " + newNick + " :Nickname set successfully\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}

/*
Data Buffer:
reply.c_str() is a pointer to the start of the string reply.

reply.size() is the length of the string, specifying how many bytes from the
buffer should be sent.

Data Transmission:
The system call send sends data from the specified buffer (reply.c_str())
through the socket identified by the file descriptor fd.

Return Value:
send returns the number of bytes successfully sent to the client.
If an error occurs, it returns -1, and additional error information can be
retrieved using errno.

*/