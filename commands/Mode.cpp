#include "Mode.hpp"
#include "../include/Server.hpp"
#include <sys/socket.h>
#include <string>
#include <exception>

/**
 * @brief Handles the MODE command from a client.
 *
 * This function processes the MODE command, which allows a client to change channel modes.
 * The command requires at least three parameters: MODE, channel name, and the mode string.
 * Depending on the specified mode character, additional parameters may be required:
 *   - 'k': channel key (password), requires an additional parameter.
 *   - 'l': user limit, requires an additional parameter that should be convertible to a number.
 *   - 'o': operator mode, requires an additional parameter (target nickname).
 *
 * For each mode, the function performs appropriate validation (such as checking for sufficient parameters,
 * verifying numeric conversion for mode 'l', etc.). After processing, it sends notifications to the other
 * members of the channel.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the MODE command.
 * @param tokens A vector of strings representing the tokenized command (e.g., ["MODE", "<channel>", "<mode>", "<param>"]).
 * @param command The complete command string (unused in the implementation).
 */
void handleModeCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) {
    (void)command; // Suppress unused parameter warning

    // Check if the client is fully registered.
    if (server->_clients[fd]->authState != AUTH_REGISTERED) {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Ensure there are at least three parameters: MODE <channel> <mode>[ <param>]
    if (tokens.size() < 3) {
        std::string reply = "461 MODE :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Extract channel name and mode string from the tokens.
    std::string channelName = tokens[1];
    std::string modeStr = tokens[2];
    
    // Locate the channel in the server's channel map.
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end()) {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    // Determine if the mode should be enabled or disabled.
    bool enable = (modeStr[0] == '+');
    
    // Process each mode character in the mode string (skipping the first character '+' or '-').
    for (size_t i = 1; i < modeStr.size(); ++i) {
        char modeChar = modeStr[i];
        switch (modeChar) {
            case 'i':  // Invite-only mode.
            case 't':  // Topic restricted mode.
                it->second.setMode(modeChar, enable);
                break;
            case 'k':  // Channel key mode.
                if (enable) {
                    // When enabling mode 'k', a key must be provided.
                    if (tokens.size() >= 4) {
                        it->second.setMode('k', enable, tokens[3]);
                    } else {
                        std::string reply = "461 MODE k :Not enough parameters\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                        return;
                    }
                } else {
                    // When disabling mode 'k', clear the key.
                    it->second.setMode('k', enable, "");
                }
                break;
            case 'l':  // User limit mode.
                if (enable) {
                    // When enabling mode 'l', a numeric limit must be provided.
                    if (tokens.size() >= 4) {
                        try {
                            it->second.setMode('l', enable, tokens[3]);
                        } catch (const std::exception& e) {
                            std::string reply = "461 MODE l :Invalid limit parameter\r\n";
                            send(fd, reply.c_str(), reply.size(), 0);
                            return;
                        }
                    } else {
                        std::string reply = "461 MODE l :Not enough parameters\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                        return;
                    }
                } else {
                    // When disabling mode 'l', clear the user limit.
                    it->second.setMode('l', enable, "");
                }
                break;
            case 'o':  // Operator mode.
                if (enable) {
                    // Enabling operator mode requires the target nickname.
                    if (tokens.size() >= 4) {
                        std::string targetNick = tokens[3];
                        int targetFd = -1;
                        // Find the client by nickname.
                        for (const auto& pair : server->_clients) {
                            if (pair.second->nickname == targetNick) {
                                targetFd = pair.first;
                                break;
                            }
                        }
                        if (targetFd != -1) {
                            it->second.addOperator(targetFd);
                            std::string modeMsg = ":" + server->_clients[fd]->nickname + " MODE " + channelName + " " + modeStr + " " + tokens[3] + "\r\n";
                            // Notify other channel members.
                            for (int cli_fd : it->second.getClients()) {
                                if (cli_fd != fd)
                                    send(cli_fd, modeMsg.c_str(), modeMsg.size(), 0);
                            }
                        } else {
                            std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
                            send(fd, reply.c_str(), reply.size(), 0);
                        }
                    } else {
                        std::string reply = "461 MODE o :Not enough parameters\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                    }
                } else {
                    // Disabling operator mode; requires the target nickname.
                    if (tokens.size() >= 4) {
                        std::string targetNick = tokens[3];
                        int targetFd = -1;
                        for (const auto& pair : server->_clients) {
                            if (pair.second->nickname == targetNick) {
                                targetFd = pair.first;
                                break;
                            }
                        }
                        if (targetFd != -1) {
                            it->second.removeOperator(targetFd);
                            std::string modeMsg = ":" + server->_clients[fd]->nickname + " MODE " + channelName + " " + modeStr + " " + tokens[3] + "\r\n";
                            for (int cli_fd : it->second.getClients()) {
                                if (cli_fd != fd)
                                    send(cli_fd, modeMsg.c_str(), modeMsg.size(), 0);
                            }
                        } else {
                            std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
                            send(fd, reply.c_str(), reply.size(), 0);
                        }
                    } else {
                        std::string reply = "461 MODE o :Not enough parameters\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                    }
                }
                break;
            default:
                // Unknown mode character; optionally ignore or send an error.
                break;
        }
    }
    
    // After applying mode changes, send a notification to all channel members.
    std::string modeMsg = ":" + server->_clients[fd]->nickname + " MODE " + channelName + " " + modeStr;
    if (tokens.size() >= 4 && (modeStr.find('k') != std::string::npos || modeStr.find('l') != std::string::npos))
        modeMsg += " " + tokens[3];
    modeMsg += "\r\n";
    
    for (int cli_fd : it->second.getClients()) {
        if (cli_fd != fd)
            send(cli_fd, modeMsg.c_str(), modeMsg.size(), 0);
    }
}
