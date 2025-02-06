/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 11:31:13 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/05 12:35:36 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"
#include "../include/Server.hpp"
#include <string>

/**
 * @brief Handles the USER command from a client.
 *
 * This function processes the USER command, which is used to set user information after a client has
 * connected to the server. It expects at least five tokens in the command. The second token is used as
 * the client's username. If the client's nickname has already been set (by the NICK command), then the
 * client's authentication state is updated to AUTH_REGISTERED. Finally, a confirmation message is sent to
 * the client indicating that the user information has been successfully set.
 *
 * @param server Pointer to the Server object that manages the IRC server.
 * @param fd The file descriptor of the client issuing the USER command.
 * @param tokens A vector of strings representing the tokenized command arguments. Expected format is:
 *               "USER" <username> <...> (other parameters are ignored in this implementation).
 * @param command The complete command string received from the client (unused in this implementation).
 */

void handleUserCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& /*command*/) {
    // Ensure that the command contains at least five parameters.
    if (tokens.size() < 5) {
        std::string reply = "461 USER :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Set the client's username from the second token.
    server->getClients()[fd]->setUsername(tokens[1]);
    
    // If the client's nickname has already been set (via the NICK command), mark the client as fully registered.
    if (!server->getClients()[fd]->getNickname().empty())
        server->getClients()[fd]->authState = AUTH_REGISTERED;
    
    // Send a confirmation message to the client indicating that the user information is set successfully.
    std::string reply = "001 " + server->getClients()[fd]->getNickname() + " :User information set successfully\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}
