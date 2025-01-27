#ifndef JOIN_HPP
#define JOIN_HPP

#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the JOIN command from a client.
 *
 * This function processes the JOIN command, which allows a client to join a specified channel.
 * It validates that the client is fully registered, verifies that the necessary parameters
 * are provided, creates the channel if it doesn't exist, and performs additional checks such as
 * invite-only mode and user limit restrictions. If all conditions are met, the client is added
 * to the channel and a JOIN notification is sent to existing channel members.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the JOIN command.
 * @param tokens A vector containing the tokenized command arguments (expected: "JOIN" and <channelName>).
 * @param command The complete command string received from the client.
 */
void handleJoinCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // JOIN_HPP
