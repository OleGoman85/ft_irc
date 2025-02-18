#ifndef USER_HPP
#define USER_HPP
#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the USER command from a client.
 *
 * This function processes the USER command, which is used to set the client's user information
 * (typically after a NICK command has been issued). It expects at least five parameters in the command.
 * The second parameter is used as the client's username. If the client's nickname is already set, 
 * the client's authentication state is updated to fully registered. Finally, a confirmation 
 * message is sent back to the client.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the USER command.
 * @param tokens A vector of strings representing the tokenized command arguments. 
 *               The expected format is: "USER" <username> ... (other parameters are accepted but ignored).
 * @param command The complete command string received from the client.
 */
void handleUserCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // USER_HPP
