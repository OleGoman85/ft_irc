/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarbenin <aarbenin@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 11:31:13 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/11 14:49:11 by aarbenin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

#include <string>

#include "../include/Replies.hpp"
#include "../include/Server.hpp"

/**
 * @brief Handles the USER command from a client.
 *
 * This function processes the USER command, which is used to set user
 * information after a client has connected to the server. It expects at least
 * five tokens in the command. The second token is used as the client's
 * username. If the client's nickname has already been set (by the NICK
 * command), then the client's authentication state is updated to
 * AUTH_REGISTERED. Finally, a confirmation message is sent to the client
 * indicating that the user information has been successfully set.
 *
 * @param server Pointer to the Server object that manages the IRC server.
 * @param fd The file descriptor of the client issuing the USER command.
 * @param tokens A vector of strings representing the tokenized command
 * arguments. Expected format is: "USER" <username> <...> (other parameters are
 * ignored in this implementation).
 * @param command The complete command string received from the client (unused
 * in this implementation).
 */

void handleUserCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    if (tokens.size() < 5)
    {
        std::string reply = "461 USER :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    server->getClients()[fd]->setUsername(tokens[1]);

    AuthState& st = server->getClients()[fd]->authState;
    if (st == WAITING_FOR_USER)
    {
        st = AUTH_REGISTERED;
        sendWelcome(server, fd);
    }
}
