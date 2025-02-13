/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alisa <alisa@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 10:44:47 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/11 18:52:21 by alisa            ###   ########.fr       */
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
    if (server->getClients().find(fd) == server->getClients().end())
        return;
    Client* c = server->getClients()[fd].get();
    std::string nick = c->getNickname();
    std::string user = c->getUsername();
    if (user.empty())
        user = "unknown";
    std::string host = c->getHost();
    std::string prefix = ":" + nick + "!" + user + "@" + host;
    std::string quitMsg = prefix + " QUIT :Client has quit\r\n";
    for (const auto& pair : server->getClients()) {
        if (pair.first != fd)
            send(pair.first, quitMsg.c_str(), quitMsg.size(), 0);
    }
    server->removeClient(fd);
}
