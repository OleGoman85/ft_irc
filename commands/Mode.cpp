#include "Mode.hpp"
#include "../include/Server.hpp"
#include <sys/socket.h>
#include <string>
#include <exception>

// /**
//  * @brief Handles the MODE command from a client.
//  *
//  * This function processes the MODE command, which allows a client to change channel modes.
//  * The command requires at least three parameters: MODE, channel name, and the mode string.
//  * Depending on the specified mode character, additional parameters may be required:
//  *   - 'k': channel key (password), requires an additional parameter.
//  *   - 'l': user limit, requires an additional parameter that should be convertible to a number.
//  *   - 'o': operator mode, requires an additional parameter (target nickname).
//  *
//  * For each mode, the function performs appropriate validation (such as checking for sufficient parameters,
//  * verifying numeric conversion for mode 'l', etc.). After processing, it sends notifications to the other
//  * members of the channel.
//  *
//  * @param server Pointer to the Server object managing the IRC server.
//  * @param fd The file descriptor of the client issuing the MODE command.
//  * @param tokens A vector of strings representing the tokenized command (e.g., ["MODE", "<channel>", "<mode>", "<param>"]).
//  * @param command The complete command string (unused in the implementation).
//  */
// void handleModeCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) {
//     (void)command; // Suppress unused parameter warning

//     // Check if the client is fully registered.
//     if (server->_clients[fd]->authState != AUTH_REGISTERED) {
//         std::string reply = "451 :You have not registered\r\n";
//         send(fd, reply.c_str(), reply.size(), 0);
//         return;
//     }
    
//     // Ensure there are at least three parameters: MODE <channel> <mode>[ <param>]
//     if (tokens.size() < 3) {
//         std::string reply = "461 MODE :Not enough parameters\r\n";
//         send(fd, reply.c_str(), reply.size(), 0);
//         return;
//     }
    
//     // Extract channel name and mode string from the tokens.
//     std::string channelName = tokens[1];
//     std::string modeStr = tokens[2];
    
//     // Locate the channel in the server's channel map.
//     auto it = server->_channels.find(channelName);
//     if (it == server->_channels.end()) {
//         std::string reply = "403 " + channelName + " :No such channel\r\n";
//         send(fd, reply.c_str(), reply.size(), 0);
//         return;
//     }
    
//     // Determine if the mode should be enabled or disabled.
//     bool enable = (modeStr[0] == '+');
    
//     // Process each mode character in the mode string (skipping the first character '+' or '-').
//     for (size_t i = 1; i < modeStr.size(); ++i) {
//         char modeChar = modeStr[i];
//         switch (modeChar) {
//             case 'i':  // Invite-only mode.
//             case 't':  // Topic restricted mode.
//                 it->second.setMode(modeChar, enable);
//                 break;
//             case 'k':  // Channel key mode.
//                 if (enable) {
//                     // When enabling mode 'k', a key must be provided.
//                     if (tokens.size() >= 4) {
//                         it->second.setMode('k', enable, tokens[3]);
//                     } else {
//                         std::string reply = "461 MODE k :Not enough parameters\r\n";
//                         send(fd, reply.c_str(), reply.size(), 0);
//                         return;
//                     }
//                 } else {
//                     // When disabling mode 'k', clear the key.
//                     it->second.setMode('k', enable, "");
//                 }
//                 break;
//             case 'l':  // User limit mode.
//                 if (enable) {
//                     // When enabling mode 'l', a numeric limit must be provided.
//                     if (tokens.size() >= 4) {
//                         try {
//                             it->second.setMode('l', enable, tokens[3]);
//                         } catch (const std::exception& e) {
//                             std::string reply = "461 MODE l :Invalid limit parameter\r\n";
//                             send(fd, reply.c_str(), reply.size(), 0);
//                             return;
//                         }
//                     } else {
//                         std::string reply = "461 MODE l :Not enough parameters\r\n";
//                         send(fd, reply.c_str(), reply.size(), 0);
//                         return;
//                     }
//                 } else {
//                     // When disabling mode 'l', clear the user limit.
//                     it->second.setMode('l', enable, "");
//                 }
//                 break;
//             case 'o':  // Operator mode.
//                 if (enable) {
//                     // Enabling operator mode requires the target nickname.
//                     if (tokens.size() >= 4) {
//                         std::string targetNick = tokens[3];
//                         int targetFd = -1;
//                         // Find the client by nickname.
//                         for (const auto& pair : server->_clients) {
//                             if (pair.second->nickname == targetNick) {
//                                 targetFd = pair.first;
//                                 break;
//                             }
//                         }
//                         if (targetFd != -1) {
//                             it->second.addOperator(targetFd);
//                             std::string modeMsg = ":" + server->_clients[fd]->nickname + " MODE " + channelName + " " + modeStr + " " + tokens[3] + "\r\n";
//                             // Notify other channel members.
//                             for (int cli_fd : it->second.getClients()) {
//                                 if (cli_fd != fd)
//                                     send(cli_fd, modeMsg.c_str(), modeMsg.size(), 0);
//                             }
//                         } else {
//                             std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
//                             send(fd, reply.c_str(), reply.size(), 0);
//                         }
//                     } else {
//                         std::string reply = "461 MODE o :Not enough parameters\r\n";
//                         send(fd, reply.c_str(), reply.size(), 0);
//                     }
//                 } else {
//                     // Disabling operator mode; requires the target nickname.
//                     if (tokens.size() >= 4) {
//                         std::string targetNick = tokens[3];
//                         int targetFd = -1;
//                         for (const auto& pair : server->_clients) {
//                             if (pair.second->nickname == targetNick) {
//                                 targetFd = pair.first;
//                                 break;
//                             }
//                         }
//                         if (targetFd != -1) {
//                             it->second.removeOperator(targetFd);
//                             std::string modeMsg = ":" + server->_clients[fd]->nickname + " MODE " + channelName + " " + modeStr + " " + tokens[3] + "\r\n";
//                             for (int cli_fd : it->second.getClients()) {
//                                 if (cli_fd != fd)
//                                     send(cli_fd, modeMsg.c_str(), modeMsg.size(), 0);
//                             }
//                         } else {
//                             std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
//                             send(fd, reply.c_str(), reply.size(), 0);
//                         }
//                     } else {
//                         std::string reply = "461 MODE o :Not enough parameters\r\n";
//                         send(fd, reply.c_str(), reply.size(), 0);
//                     }
//                 }
//                 break;
//             default:
//                 // Unknown mode character; optionally ignore or send an error.
//                 break;
//         }
//     }
    
//     // After applying mode changes, send a notification to all channel members.
//     std::string modeMsg = ":" + server->_clients[fd]->nickname + " MODE " + channelName + " " + modeStr;
//     if (tokens.size() >= 4 && (modeStr.find('k') != std::string::npos || modeStr.find('l') != std::string::npos))
//         modeMsg += " " + tokens[3];
//     modeMsg += "\r\n";
    
//     for (int cli_fd : it->second.getClients()) {
//         if (cli_fd != fd)
//             send(cli_fd, modeMsg.c_str(), modeMsg.size(), 0);
//     }
// }












#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include <sstream>
#include <stdexcept>

/**
 * @brief Determines if a mode character requires a parameter.
 *
 * @param modeChar The mode character (e.g. 'k', 'l', 'o', etc.)
 * @return true if the mode requires a parameter, false otherwise.
 */
bool modeRequiresParam(char modeChar) {
    // 'k' (key), 'l' (limit), 'o' (operator) all require params
    // 'i' (invite-only), 't' (topic-restricted) do not require params
    switch (modeChar) {
        case 'k': // channel key
        case 'l': // user limit
        case 'o': // operator
            return true;
        default:
            return false;
    }
}

/**
 * @brief Handles the MODE command for channel modes (i, t, k, l, o).
 *
 * Format examples:
 *  MODE #channel         -> show current modes
 *  MODE #channel +i      -> set invite-only
 *  MODE #channel -i      -> unset invite-only
 *  MODE #channel +k key  -> set channel key
 *  MODE #channel -k      -> remove channel key
 *  MODE #channel +l 10   -> set user limit 10
 *  MODE #channel -l      -> remove user limit
 *  MODE #channel +o nick -> give operator to 'nick'
 *  MODE #channel -o nick -> remove operator from 'nick'
 *  MODE #channel +ik secret  -> set invite-only and channel key
 *  MODE #channel +lo 10 Bob  -> set limit=10 and give +o to Bob
 *
 * If the user just does MODE #channel (no second param),
 * we display the current modes.
 */
void handleModeCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& rawCommand)
{
    (void)rawCommand; // unused in this example, but could parse trailing text

    // 1️ Check if client is registered
    if (server->_clients[fd]->authState != AUTH_REGISTERED) {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 2️ At least "MODE <channel>"
    if (tokens.size() < 2) {
        std::string reply = "461 MODE :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string channelName = tokens[1];

    // 3️ Find the channel or error
    auto it = server->_channels.find(channelName);
    if (it == server->_channels.end()) {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    Channel& channel = it->second;

    // 4️ If only "MODE #channel" with no modes, show current modes
    if (tokens.size() == 2) {
        // Build a mode string (e.g. "+it" or "-k" etc.)
        std::string modes;
        if (channel.isInviteOnly())     modes += "i";
        if (channel.isTopicRestricted())modes += "t";
        if (channel.hasMode('k'))       modes += "k";
        if (channel.hasMode('l'))       modes += "l";
        // 'o' is not typically shown as a channel-wide mode,
        // it's per-user, but some servers do store a flag 'o' as well.

        // For demonstration, let's assume we store them in the channel's _modes map
        // If no modes set, at least output a "+"
        if (!modes.empty()) {
            modes = "+" + modes;
        } else {
            modes = "+";
        }

        // We might also want to show arguments (key, limit, etc.)
        // e.g. :server 324 <nick> <channel> <modes> [key or limit]
        std::string reply = "324 " + server->_clients[fd]->nickname + " " +
                            channelName + " " + modes;
        if (channel.hasMode('k')) {
            reply += " " + channel.getChannelKey();
        } else if (channel.hasMode('l')) {
            // show the limit
            reply += " " + std::to_string(channel.getUserLimit());
        }
        reply += "\r\n";

        send(fd, reply.c_str(), reply.size(), 0);

        // Also send 329 <nick> <channel> <creationTime> in real IRC,
        // but let's skip that for simplicity.
        return;
    }

    // 5️ If user tries to modify modes, check if they're channel operator
    if (!channel.isOperator(fd)) {
        std::string reply = "482 " + channelName + " :You're not a channel operator\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 6️ tokens[2] is the modes string (like "+ik" or "-l" etc.)
    std::string modeStr = tokens[2];
    // We'll parse each char in modeStr, ignoring the initial + or -
    bool enable = (modeStr[0] == '+');
    size_t paramIndex = 3; // next token might be for 'k', 'l', or 'o'

    // We keep track of final output string to broadcast
    // We'll reconstruct something like "+ik secret Bob" if we had multi-mode changes
    std::ostringstream appliedModes;
    appliedModes << (enable ? "+" : "-");

    std::vector<std::string> appliedParams;

    // 7️ Process each mode char
    for (size_t i = 1; i < modeStr.size(); ++i) {
        char modeChar = modeStr[i];

        switch (modeChar) {
            case 'i':
            case 't':
                channel.setMode(modeChar, enable);
                appliedModes << modeChar; // record it
                break;

            case 'k': {
                if (enable) {
                    // 'k' requires a key param
                    if (paramIndex < tokens.size()) {
                        std::string key = tokens[paramIndex++];
                        channel.setMode('k', true, key);
                        appliedModes << 'k';
                        appliedParams.push_back(key);
                    } else {
                        std::string reply = "461 MODE k :Not enough parameters\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                        return;
                    }
                } else {
                    // disabling 'k'
                    channel.setMode('k', false, "");
                    appliedModes << 'k';
                }
                break;
            }

            case 'l': {
                if (enable) {
                    // 'l' requires a limit
                    if (paramIndex < tokens.size()) {
                        std::string limitStr = tokens[paramIndex++];
                        try {
                            channel.setMode('l', true, limitStr);
                            appliedModes << 'l';
                            appliedParams.push_back(limitStr);
                        } catch (...) {
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
                    // disabling 'l'
                    channel.setMode('l', false, "");
                    appliedModes << 'l';
                }
                break;
            }

            case 'o': {
                // 'o' requires a nick param
                if (paramIndex < tokens.size()) {
                    std::string targetNick = tokens[paramIndex++];
                    // find the client's fd
                    int targetFd = -1;
                    for (auto& pair : server->_clients) {
                        if (pair.second->nickname == targetNick) {
                            targetFd = pair.first;
                            break;
                        }
                    }
                    if (targetFd == -1) {
                        std::string reply = "401 " + targetNick + " :No such nick\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                    } else {
                        if (enable) {
                            channel.addOperator(targetFd);
                        } 
                        else
                        {
                            if (channel.isOperator(targetFd))
                            {
                                int operatorCount = 0;
                                for (int clientFd : channel.getClients())
                                {
                                    if (channel.isOperator(clientFd))
                                        operatorCount++;
                                }
                                if (operatorCount > 1)
                                    channel.removeOperator(targetFd);
                                else
                                {
                                    std::string reply = "482 " + channelName + " :Cannot remove last operator\r\n";
                                    send(fd, reply.c_str(), reply.size(), 0);
                                    return;
                                }
                            }
                        }
                        appliedModes << 'o';
                        appliedParams.push_back(targetNick);
                    }
                } else {
                    std::string reply = "461 MODE o :Not enough parameters\r\n";
                    send(fd, reply.c_str(), reply.size(), 0);
                    return;
                }
                break;
            }

            default:
                // unknown mode, ignore or send an error
                // for now, let's ignore
                break;
        }
    }

    // 8️ Build the final broadcast string
    // e.g. ":Alice MODE #chan +ik secret"
    std::ostringstream broadcast;
    broadcast << ":" << server->_clients[fd]->nickname
              << " MODE " << channelName << " "
              << appliedModes.str();

    // If we have any parameters to show, join them with space
    for (auto& p : appliedParams) {
        broadcast << " " << p;
    }
    broadcast << "\r\n";

    std::string modeMsg = broadcast.str();

    // 9️ Send to all channel members
    for (int memberFd : channel.getClients()) {
        send(memberFd, modeMsg.c_str(), modeMsg.size(), 0);
    }
}
