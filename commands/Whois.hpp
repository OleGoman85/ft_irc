#ifndef WHOIS_HPP
#define WHOIS_HPP

#include <string>
#include <vector>

class Server;

/**
 * @brief Handles the WHOIS command.
 *
 * Returns information about the specified user.
 *
 * @param server Pointer to the Server object.
 * @param fd File descriptor of the requesting client.
 * @param tokens Tokenized command arguments.
 * @param command The complete command string.
 */
void handleWhoisCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // WHOIS_HPP
