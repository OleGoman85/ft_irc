#ifndef PRIVMSG_HPP
#define PRIVMSG_HPP
#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the PRIVMSG command.
 *
 * This function processes the PRIVMSG command, which allows a client to send a private message
 * to another user or broadcast a message to a channel. It performs validation checks such as
 * ensuring the client is fully registered and that the correct parameters are provided. In the
 * case of a channel message (target begins with '#'), it verifies that the sender is a member of
 * the channel before forwarding the message to all channel members (except the sender). For a
 * private message, it sends the message directly to the target client.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the PRIVMSG command.
 * @param tokens A vector of strings representing the tokenized command arguments.
 *               Expected format: "PRIVMSG", <target>, followed by the message text after a ':'.
 * @param command The complete command string received from the client.
 */
void handlePrivmsgCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // PRIVMSG_HPP
