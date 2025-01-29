/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 10:44:47 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/16 10:00:58 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Quit.hpp"
#include "../include/Server.hpp"
#include <string>

/**
 * @brief Handles the QUIT command from a client.
 *
 * This function processes the QUIT command, which is issued by a client to disconnect from the server.
 * It first checks if the client exists in the server's client map. If so, it constructs a QUIT message 
 * (indicating that the client has quit) and sends it to all other connected clients. Finally, it removes 
 * the client from the server.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client that issued the QUIT command.
 * @param tokens A vector of strings representing the tokenized command arguments (unused in this implementation).
 * @param command The complete command string received from the client (unused in this implementation).
 */
void handleQuitCommand(Server* server, int fd, const std::vector<std::string>& /*tokens*/, const std::string& /*command*/) {
    // Check if the client exists in the server's client map.
    if (server->_clients.find(fd) == server->_clients.end())
        return;
    
    // Construct the QUIT message with the client's nickname.
    std::string quitMsg = ":" + server->_clients[fd]->nickname + " QUIT :Client has quit\r\n";
    
    // Send the QUIT message to all other connected clients.
    for (const auto& pair : server->_clients) {
        if (pair.first != fd)
            send(pair.first, quitMsg.c_str(), quitMsg.size(), 0);
    }
    
    // Remove the client from the server.
    server->removeClient(fd);
}
