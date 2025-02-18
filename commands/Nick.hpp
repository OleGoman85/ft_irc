#ifndef NICK_HPP
#define NICK_HPP
#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the NICK command.
 *
 * This function processes the NICK command, which allows a client to set or change their nickname.
 * It ensures that a nickname is provided, checks for duplicate nicknames among connected clients,
 * updates the client's nickname, and adjusts the client's authentication state accordingly.
 * A confirmation message is then sent back to the client.
 *
 * @param server Pointer to the Server object that manages the IRC server.
 * @param fd The file descriptor of the client issuing the NICK command.
 * @param tokens A vector of strings representing the tokenized command arguments (expected format: "NICK" followed by <newNick>).
 * @param command The complete command string received from the client.
 */
void handleNickCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // NICK_HPP
