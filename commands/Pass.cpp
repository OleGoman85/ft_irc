#include "Pass.hpp"

#include <string>

#include "../include/Server.hpp"

/**
 * @brief Handles the PASS command from a client.
 *
 * This function processes the PASS command, which is used by a client to
 * authenticate with the server. It checks that the required password parameter
 * is provided, compares it with the server's password, and, if they match,
 * updates the client's authentication state to WAITING_FOR_NICK. If the
 * password is incorrect or if not enough parameters are provided, the
 * appropriate error message is sent, and in the case of an incorrect password,
 * the client is removed.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the PASS command.
 * @param tokens A vector of strings representing the tokenized command
 * arguments (expected: "PASS" followed by <password>).
 * @param command The complete command string received from the client (unused
 * in this implementation).
 */
void handlePassCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    if (tokens.size() < 2)
    {
        std::string reply = "461 PASS :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    if (tokens[1] != server->getPassword())
    {
        std::string reply = "464 PASS :Password incorrect\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        server->removeClient(fd);
        return;
    }

    server->getClients()[fd]->authState = WAITING_FOR_NICK;
}
