/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 10:43:39 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/16 09:58:56 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Pass.hpp"
#include "../include/Server.hpp"
#include <string>

/**
 * @brief Handles the PASS command from a client.
 *
 * This function processes the PASS command, which is used by a client to authenticate with the server.
 * It checks that the required password parameter is provided, compares it with the server's password, and,
 * if they match, updates the client's authentication state to WAITING_FOR_NICK.
 * If the password is incorrect or if not enough parameters are provided, the appropriate error message is sent,
 * and in the case of an incorrect password, the client is removed.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the PASS command.
 * @param tokens A vector of strings representing the tokenized command arguments (expected: "PASS" followed by <password>).
 * @param command The complete command string received from the client (unused in this implementation).
 */
void handlePassCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& /*command*/) {
    // Check if enough parameters are provided.
    if (tokens.size() < 2) {
        std::string reply = "461 PASS :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Verify that the provided password matches the server's password.
    if (tokens[1] != server->_password) {
        std::string reply = "464 PASS :Password incorrect\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        server->removeClient(fd);  // Remove client if authentication fails.
        return;
    }
    
    // If the client is not registered yet, update its authentication state.
    if (server->_clients.find(fd) != server->_clients.end()) {
        server->_clients[fd]->authState = WAITING_FOR_NICK;
        std::string reply = "001 :Password accepted.\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
    }
}
