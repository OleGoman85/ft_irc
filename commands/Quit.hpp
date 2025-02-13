#ifndef QUIT_HPP
#define QUIT_HPP

#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the QUIT command from a client.
 *
 * This function processes the QUIT command, which indicates that a client
 * wishes to disconnect from the server. It verifies that the client exists,
 * broadcasts a QUIT message (including the client's nickname) to all other
 * connected clients, and then removes the client from the server's client list.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the QUIT command.
 * @param tokens A vector of tokenized command arguments (unused in this
 * implementation).
 * @param command The complete command string received from the client (unused
 * in this implementation).
 */
void handleQuitCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string&              command);

#endif  // QUIT_HPP
