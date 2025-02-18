#ifndef MODE_HPP
#define MODE_HPP
#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the MODE command from a client.
 *
 * This function processes the MODE command, which allows a client to change
 * channel modes (such as invite-only, topic restrictions, channel key, user limit, and operator privileges).
 * The command requires at least three parameters: "MODE", the channel name, and the mode string.
 * Depending on the mode character, additional parameters may be required (e.g., channel key for 'k',
 * user limit for 'l', or target nickname for 'o').
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the MODE command.
 * @param tokens A vector of strings representing the tokenized command (e.g., ["MODE", "<channel>", "<mode>", "[<param>]"]).
 * @param command The complete command string received from the client.
 */
void handleModeCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // MODE_HPP
