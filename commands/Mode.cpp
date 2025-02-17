#include <sys/socket.h>  // for send()

#include <cctype>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

/**
 * @brief Structure to store a channel mode change.
 */
struct ModeChange
{
    bool add;   ///< true if mode is being added ('+'), false if removed ('-')
    char mode;  ///< mode character
    std::string param;  ///< parameter if needed
};

/**
 * @brief Sends a reply message to a client.
 *
 * @param fd Client's file descriptor.
 * @param message The message to send.
 */
static void sendReply(int fd, const std::string& message)
{
    send(fd, message.c_str(), message.size(), 0);
}

/**
 * @brief Checks if the client is registered.
 *
 * If not, sends the appropriate error message.
 *
 * @param server Pointer to the server.
 * @param fd Client's file descriptor.
 * @return true if registered, false otherwise.
 */
static bool checkRegistration(Server* server, int fd)
{
    // Using operator-> of unique_ptr works as usual.
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        sendReply(fd, "451 :You have not registered\r\n");
        return false;
    }
    return true;
}

/**
 * @brief Retrieves a pointer to the channel by name.
 *
 * If the channel does not exist, sends an error message.
 *
 * @param server Pointer to the server.
 * @param fd Client's file descriptor.
 * @param channelName The name of the channel.
 * @return Pointer to the Channel if found, otherwise NULL.
 */
static Channel* getChannel(Server* server, int fd,
                           const std::string& channelName)
{
    std::map<std::string, Channel>&          channels = server->getChannels();
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it == channels.end())
    {
        sendReply(fd, "403 " + channelName + " :No such channel\r\n");
        return NULL;
    }
    return &(it->second);
}

/**
 * @brief Prints the current modes of the channel to the requesting client.
 *
 * @param server Pointer to the server.
 * @param fd Client's file descriptor.
 * @param channel Reference to the channel.
 * @param channelName The name of the channel.
 */
static void printCurrentModes(Server* server, int fd, Channel& channel,
                              const std::string& channelName)
{
    std::string modes;
    if (channel.isInviteOnly()) modes += "i";
    if (channel.isTopicRestricted()) modes += "t";
    if (channel.hasMode('k')) modes += "k";
    if (channel.hasMode('l')) modes += "l";

    if (modes.empty())
        modes = "+";
    else
        modes = "+" + modes;

    std::ostringstream reply;
    reply << "324 " << server->getClients()[fd]->getNickname() << " "
          << channelName << " " << modes;
    if (channel.hasMode('k')) reply << " " << channel.getChannelKey();
    if (channel.hasMode('l'))
        reply << " " << std::to_string(channel.getUserLimit());
    reply << "\r\n";
    sendReply(fd, reply.str());
}

/**
 * @brief Parses the mode string and applies the changes to the channel.
 *
 * For each mode flag, validates parameters, applies the change via the Channel
 * interface, and records the change in the changes vector.
 *
 * @param server Pointer to the server.
 * @param fd Client's file descriptor.
 * @param channel Reference to the channel.
 * @param modeStr The mode string (e.g. "+ikl-t").
 * @param tokens The vector of command tokens.
 * @param paramIdx Index in tokens where parameters start.
 * @param changes Vector to store applied mode changes.
 * @return true if parsing and application succeeded, false otherwise.
 */
static bool parseAndApplyModeChanges(Server* server, int fd, Channel& channel,
                                     const std::string&              modeStr,
                                     const std::vector<std::string>& tokens,
                                     size_t&                         paramIdx,
                                     std::vector<ModeChange>&        changes)
{
    // Validate that modeStr starts with '+' or '-'
    if (modeStr.empty() || (modeStr[0] != '+' && modeStr[0] != '-'))
    {
        sendReply(fd, "472 " + server->getClients()[fd]->getNickname() +
                          " :Invalid mode string\r\n");
        return false;
    }

    bool currentSign = (modeStr[0] == '+');
    // Process each character in the mode string.
    for (size_t i = 0; i < modeStr.size(); ++i)
    {
        char c = modeStr[i];
        if (c == '+')
        {
            currentSign = true;
            continue;
        }
        if (c == '-')
        {
            currentSign = false;
            continue;
        }

        ModeChange change;
        change.add  = currentSign;
        change.mode = c;

        switch (c)
        {
            case 'i':
            case 't':
            {
                // Modes 'i' and 't' do not require parameters.
                channel.setMode(c, currentSign);
                changes.push_back(change);
                break;
            }
            case 'k':
            {
                // Mode 'k' (channel key) requires a parameter when adding.
                if (currentSign)
                {
                    if (paramIdx >= tokens.size())
                    {
                        sendReply(fd,
                                  "461 MODE :Not enough parameters for +k\r\n");
                        return false;
                    }
                    std::string key = tokens[paramIdx++];
                    channel.setMode('k', true, key);
                    change.param = key;
                    changes.push_back(change);
                }
                else
                {
                    channel.setMode('k', false);
                    changes.push_back(change);
                }
                break;
            }
            case 'l':
            {
                // Mode 'l' (user limit) requires a numeric parameter when
                // adding.
                if (currentSign)
                {
                    if (paramIdx >= tokens.size())
                    {
                        sendReply(fd,
                                  "461 MODE :Not enough parameters for +l\r\n");
                        return false;
                    }
                    std::string limitStr = tokens[paramIdx++];
                    try
                    {
                        int limit = std::stoi(limitStr);
                        if (limit <= 0)
                        {
                            sendReply(
                                fd, "461 MODE l :Invalid limit parameter\r\n");
                            return false;
                        }
                        channel.setMode('l', true, limitStr);
                        change.param = limitStr;
                        changes.push_back(change);
                    }
                    catch (const std::exception&)
                    {
                        sendReply(fd,
                                  "461 MODE l :Invalid limit parameter\r\n");
                        return false;
                    }
                }
                else
                {
                    channel.setMode('l', false);
                    changes.push_back(change);
                }
                break;
            }
            case 'o':
            {
                // Mode 'o' (channel operator) requires a target nickname.
                if (paramIdx >= tokens.size())
                {
                    sendReply(fd,
                              "461 MODE :Not enough parameters for +o/-o\r\n");
                    return false;
                }
                std::string targetNick = tokens[paramIdx++];
                int         targetFd   = -1;
                // Note: getClients() returns a map<int,
                // std::unique_ptr<Client>>
                std::map<int, std::unique_ptr<Client> >& clients =
                    server->getClients();
                for (std::map<int, std::unique_ptr<Client> >::iterator it =
                         clients.begin();
                     it != clients.end(); ++it)
                {
                    if (it->second->getNickname() == targetNick)
                    {
                        targetFd = it->first;
                        break;
                    }
                }
                if (targetFd == -1)
                {
                    sendReply(fd, "401 " + targetNick + " :No such nick\r\n");
                    return false;
                }
                if (!channel.hasClient(targetFd))
                {
                    sendReply(fd, "441 " + targetNick + " " +
                                      channel.getName() +
                                      " :They aren't on that channel\r\n");
                    return false;
                }
                if (currentSign)
                {
                    channel.addOperator(targetFd);
                }
                else
                {
                    if (channel.isOperator(targetFd))
                    {
                        int              opCount        = 0;
                        std::vector<int> channelClients = channel.getClients();
                        for (std::vector<int>::iterator it =
                                 channelClients.begin();
                             it != channelClients.end(); ++it)
                        {
                            if (channel.isOperator(*it)) ++opCount;
                        }
                        if (opCount > 1)
                        {
                            channel.removeOperator(targetFd);
                        }
                        else
                        {
                            sendReply(
                                fd,
                                "482 " + channel.getName() +
                                    " :Cannot remove the last operator\r\n");
                            return false;
                        }
                    }
                    // If target is not an operator, nothing to do.
                }
                change.param = targetNick;
                changes.push_back(change);
                break;
            }
            default:
            {
                // Unknown mode character.
                sendReply(fd, "472 " + server->getClients()[fd]->getNickname() +
                                  " " + std::string(1, c) +
                                  " :is unknown mode char to me\r\n");
                // Do not add unknown mode to changes.
                break;
            }
        }
    }
    return true;
}

/**
 * @brief Broadcasts the mode change to all clients in the channel.
 *
 * Constructs a grouped mode string and sends the broadcast message.
 *
 * @param server Pointer to the server.
 * @param sourceFd The file descriptor of the client who issued the command.
 * @param channel Reference to the channel.
 * @param changes Vector of mode changes applied.
 */
static void broadcastModeChange(Server* server, int sourceFd, Channel& channel,
                                const std::vector<ModeChange>& changes)
{
    if (changes.empty()) return;

    std::ostringstream       modeStream;
    std::vector<std::string> modeParams;

    modeStream << (changes[0].add ? "+" : "-");
    char currentGroupSign = (changes[0].add ? '+' : '-');

    for (size_t i = 0; i < changes.size(); ++i)
    {
        if ((changes[i].add ? '+' : '-') != currentGroupSign)
        {
            modeStream << " " << (changes[i].add ? "+" : "-");
            currentGroupSign = (changes[i].add ? '+' : '-');
        }
        modeStream << changes[i].mode;
        if (!changes[i].param.empty()) modeParams.push_back(changes[i].param);
    }
    Client*     sourceClient = server->getClients()[sourceFd].get();
    std::string nick         = sourceClient->getNickname();
    std::string user         = sourceClient->getUsername();
    if (user.empty()) user = "unknown";
    std::string host = sourceClient->getHost();
    if (host.empty()) host = "localhost";

    std::string prefix = ":" + nick + "!" + user + "@" + host;

    std::ostringstream broadcast;
    broadcast << prefix << " MODE " << channel.getName() << " "
              << modeStream.str();
    for (size_t i = 0; i < modeParams.size(); ++i)
        broadcast << " " << modeParams[i];
    broadcast << "\r\n";
    std::string      msg     = broadcast.str();
    std::vector<int> clients = channel.getClients();
    for (size_t i = 0; i < clients.size(); ++i) sendReply(clients[i], msg);
}

/**
 * @brief Handles the MODE command.
 *
 * Validates the command, checks registration, retrieves the channel, and either
 * prints the current modes or parses and applies new mode changes. Finally,
 * broadcasts the changes.
 *
 * @param server Pointer to the server.
 * @param fd Client's file descriptor.
 * @param tokens Parsed command tokens.
 * @param rawCommand The raw command string.
 */
void handleModeCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string&              rawCommand)
{
    (void)rawCommand;

    if (!checkRegistration(server, fd)) return;

    if (tokens.size() < 2)
    {
        sendReply(fd, "461 MODE :Not enough parameters\r\n");
        return;
    }

    std::string channelName = tokens[1];

    if (!channelName.empty() && channelName[0] != '#')
    {
        std::string myNick = server->getClients()[fd]->getNickname();
        if (channelName != myNick)
        {
            // 502: "Cannot change mode for other users"
            sendReply(fd, "502 " + channelName + " :Cannot change mode for other users\r\n");
            return;
        }
        std::string notice = "NOTICE " + myNick + " :User modes not used on this server\r\n";
        sendReply(fd, notice);

        return;
    }

    Channel*    channel     = getChannel(server, fd, channelName);
    if (!channel) return;

    if (tokens.size() == 2)
    {
        printCurrentModes(server, fd, *channel, channelName);
        return;
    }

    if (!channel->isOperator(fd))
    {
        sendReply(fd,
                  "482 " + channelName + " :You're not a channel operator\r\n");
        return;
    }

    std::string             modeStr  = tokens[2];
    size_t                  paramIdx = 3;
    std::vector<ModeChange> changes;

    if (!parseAndApplyModeChanges(server, fd, *channel, modeStr, tokens,
                                  paramIdx, changes))
        return;

    broadcastModeChange(server, fd, *channel, changes);
}
