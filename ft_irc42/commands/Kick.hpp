#ifndef KICK_HPP
#define KICK_HPP

#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the KICK command.
 *
 * This function processes the KICK command issued by a client, which removes a target user from a specified channel.
 * It verifies that the sender is fully registered, checks that the required parameters are provided,
 * finds the channel and target client, removes the target from the channel if present, 
 * and then broadcasts a KICK notification to the remaining members of the channel.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the KICK command.
 * @param tokens A vector of strings containing the tokenized command arguments (expected format: "KICK", <channelName>, <targetNick>).
 * @param command The complete command string.
 */
void handleKickCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // KICK_HPP
