#ifndef WHO_HPP
#define WHO_HPP
#include <string>
#include <vector>

class Server;

/**
 * @brief Handles the WHO command.
 *
 * If a parameter is provided and начинается с '#', lists the users on that channel;
 * otherwise, lists all connected users.
 *
 * @param server Pointer to the Server object.
 * @param fd File descriptor of the requesting client.
 * @param tokens Tokenized command arguments.
 * @param command The complete command string.
 */
void handleWhoCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // WHO_HPP
