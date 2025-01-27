#ifndef TOPIC_HPP
#define TOPIC_HPP

#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the TOPIC command from a client.
 *
 * This function processes the TOPIC command, which is used to view or change a channel's topic.
 * When a new topic is provided (indicated by the presence of a ':' in the command), the function
 * verifies (if necessary) that the client has operator privileges when the channel is topic-restricted,
 * then updates the channel topic and notifies all channel members.
 *
 * If no new topic is provided, the current topic (if set) is sent back to the client.
 *
 * @param server Pointer to the Server object that manages the IRC server.
 * @param fd The file descriptor of the client issuing the TOPIC command.
 * @param tokens A vector of strings containing the tokenized command arguments.
 *               Expected tokens: "TOPIC", <channel>, and (optionally) the new topic text (following a colon).
 * @param command The complete command string received from the client.
 */
void handleTopicCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // TOPIC_HPP
