#ifndef LIST_HPP
#define LIST_HPP

#include <string>
#include <vector>

class Server;

/**
 * @brief Handles the LIST command.
 *
 * Lists all channels along with the number of users and the topic.
 *
 * @param server Pointer to the Server object.
 * @param fd File descriptor of the requesting client.
 * @param tokens Tokenized command arguments (необязательно используются).
 * @param command The complete command string.
 */
void handleListCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // LIST_HPP
