#include "User.hpp"
#include <string>
#include "../include/Replies.hpp"
#include "../include/Server.hpp"

/**
 * @brief Handles the USER command from a client.
 *
 * The USER command is used to set the username and real name of a client 
 * after connecting to the server. It requires at least five tokens in the command. 
 * 
 * The function performs the following steps:
 * 1. **Validate Parameters**: Ensures that at least five tokens are present.
 * 2. **Set Username**: Extracts and assigns the username from the second token.
 * 3. **Set Real Name**: Extracts the real name (if provided) from the command string.
 * 4. **Check Authentication State**:
 *    - If the client has already set their nickname (via NICK command), they are fully authenticated.
 *    - Calls `sendWelcome` to confirm the authentication.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client sending the USER command.
 * @param tokens Vector of strings representing the parsed command arguments.
 *               Expected format: `"USER" <username> <unused1> <unused2> <unused3> :<realname>`
 * @param command The full command string received from the client (unused except for real name extraction).
 */
void handleUserCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& command)
{
    // Step 1: Validate the number of parameters
    if (tokens.size() < 5)
    {
        std::string reply = "461 USER :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Step 2: Set the username (from the second token)
    server->getClients()[fd]->setUsername(tokens[1]);

    // Step 3: Extract and set the real name (everything after the colon `:`)
    size_t colonPos = command.find(':');
    std::string realName;
    if (colonPos != std::string::npos) {
        realName = command.substr(colonPos + 1); // Extract real name
    } else {
        realName = ""; // If no real name provided, set it to an empty string
    }
    server->getClients()[fd]->setRealName(realName);

    // Step 4: Update authentication state and send welcome message if needed
    AuthState& st = server->getClients()[fd]->authState;
    if (st == WAITING_FOR_USER) 
    {
        st = AUTH_REGISTERED;  // Mark client as fully registered
        sendWelcome(server, fd); // Send welcome message to confirm authentication
    }
}

