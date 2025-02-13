#ifndef CAP_HPP
#define CAP_HPP

#include <string>
#include <vector>

class Server;

/**
 * @brief Handles the CAP command for capability negotiation.
 *
 * This function processes the CAP command, which is used to negotiate and manage
 * extended capabilities between the client and the server. The following subcommands
 * are supported:
 *
 *  - CAP LS:   Request the list of capabilities supported by the server.
 *  - CAP REQ:  Request to enable specific capabilities (the server will acknowledge them).
 *  - CAP LIST: Return the list of currently active capabilities for the client.
 *  - CAP CLEAR: Clear all active capabilities.
 *  - CAP END:  End the CAP negotiation process.
 *
 * @param server  Pointer to the Server object.
 * @param fd      File descriptor of the client sending the command.
 * @param tokens  The tokenized command arguments.
 * @param command The full command string as received from the client.
 */
void handleCapCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // CAP_HPP
