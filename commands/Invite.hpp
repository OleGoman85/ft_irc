#ifndef INVITE_HPP
#define INVITE_HPP

#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the INVITE command.
 *
 * This function processes an INVITE command from a client. It checks that the sender is fully registered,
 * verifies that sufficient parameters are provided, searches for the target client and the specified channel,
 * and sends an invitation message. Additionally, if the target client is not already in the channel,
 * it is automatically added and a JOIN notification is broadcast to the channel members.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client that sent the INVITE command.
 * @param tokens The tokenized command arguments (expected: INVITE <targetNick> <channel>).
 * @param command The complete command string.
 */
void handleInviteCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // INVITE_HPP
