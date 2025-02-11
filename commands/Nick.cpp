/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarbenin <aarbenin@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 11:30:54 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/11 14:46:12 by aarbenin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Nick.hpp"

#include <string>

#include "../include/Replies.hpp"
#include "../include/Server.hpp"

/**
 * @brief Handles the NICK command from a client.
 *
 * This function processes the NICK command, which is used by a client to set or
 * change its nickname. The function first verifies that a nickname was
 * provided. It then checks if the desired nickname is already in use by another
 * client. If the nickname is available, the client's nickname is updated.
 * Additionally, if the client's authentication state indicates that they are
 * waiting for the USER command, their state is updated accordingly. A
 * confirmation message is then sent to the client.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the NICK command.
 * @param tokens A vector of strings representing the tokenized command
 * arguments (expected: "NICK" followed by <newNick>).
 * @param command The complete command string received from the client (unused
 * in this implementation).
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
        if (pair.second->getNickname() == newNick)
        {
            std::string reply =
                "433 * " + newNick + " :Nickname is already in use\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }

    server->getClients()[fd]->setNickname(newNick);

    AuthState& st = server->getClients()[fd]->authState;

    if (st == WAITING_FOR_NICK) st = WAITING_FOR_USER;

    Client* c = server->getClients()[fd].get();
    if (st == WAITING_FOR_USER && !c->getUsername().empty())
    {
        st = AUTH_REGISTERED;
        sendWelcome(server, fd);
    }
}
