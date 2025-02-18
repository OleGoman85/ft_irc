#include "../include/Channel.hpp"
#include <cstdlib> 
#include <stdexcept>

/**
 * @brief Default constructor for Channel.
 *
 * This constructor initializes a channel with default values. 
 * The channel name is empty, and all settings are set to their 
 * default states (no topic, not invite-only, no restrictions, 
 * no key, and no user limit). The mode flags are also initialized 
 * in the `_modes` map.
 */
Channel::Channel()
    : _name(""),
      _topic(""),
      _inviteOnly(false),
      _topicRestricted(false),
      _channelKey(""),
      _userLimit(0)
{
    // Initialize mode flags in the _modes map
    _modes['i'] = _inviteOnly;     // Invite-only mode
    _modes['t'] = _topicRestricted; // Topic-restricted mode
    _modes['k'] = false;           // Channel key (password) mode
    _modes['o'] = false;           // Operator mode
    _modes['l'] = false;           // User limit mode
}


/**
 * @brief Parameterized constructor for Channel.
 *
 * This constructor initializes a channel with a given name. 
 * Other properties are set to their default values (empty topic, 
 * no restrictions, no key, no user limit). The mode flags are 
 * also initialized in the `_modes` map.
 *
 * @param name The name of the channel.
 */
Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
      _inviteOnly(false),
      _topicRestricted(false),
      _channelKey(""),
      _userLimit(0)
{
    // Initialize mode flags in the _modes map
    _modes['i'] = _inviteOnly;     // Invite-only mode
    _modes['t'] = _topicRestricted; // Topic-restricted mode
    _modes['k'] = false;           // Channel key (password) mode
    _modes['o'] = false;           // Operator mode
    _modes['l'] = false;           // User limit mode
}


Channel::~Channel()
{
}


/**
 * @brief Retrieves the channel name.
 *
 * @return The name of the channel.
 */
std::string Channel::getName() const
{
    return _name;
}


/**
 * @brief Adds a client to the channel.
 *
 * Checks if the client (by file descriptor) is already a member of the channel.
 * If not, adds the client's file descriptor to the _clients vector.
 *
 * @param fd The file descriptor of the client to be added.
 */
void Channel::addClient(int fd)
{
    if (!hasClient(fd)) _clients.push_back(fd);
}


/**
 * @brief Removes a client from the channel.
 *
 * Iterates over the _clients vector and removes the client with the given file
 * descriptor. removes operator status
 *
 * @param fd The file descriptor of the client to be removed.
 */
void Channel::removeClient(int fd)
{
    for (auto it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (*it == fd)
        {
            _clients.erase(it);
            break;
        }
    }
    removeOperator(fd);
}


/**
 * @brief Checks if a client is present in the channel.
 *
 * Iterates over the _clients vector to verify if a client with the given file
 * descriptor is a member.
 *
 * @param fd The file descriptor of the client.
 * @return true if the client is in the channel; false otherwise.
 */
bool Channel::hasClient(int fd) const
{
    for (const auto& client_fd : _clients)
    {
        if (client_fd == fd) return true;
    }
    return false;
}


/**
 * @brief Sets the channel topic.
 *
 * Updates the channel's topic.
 *
 * @param topic The new topic for the channel.
 */
void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}


/**
 * @brief Retrieves the channel topic.
 *
 * @return The current topic of the channel.
 */
std::string Channel::getTopic() const
{
    return _topic;
}


/**
 * @brief Sets a mode for the channel.
 *
 * Updates the mode flag for the specified mode character and, if necessary,
 * sets a parameter. Supported modes:
 * - 'i': Invite-only mode.
 * - 't': Topic restricted mode (only operators may change topic).
 * - 'k': Channel key (password); when enabled, param contains the key.
 * - 'l': User limit; when enabled, param should be convertible to an integer.
 * - 'o': Operator mode is not handled here as operator management is done via
 * separate methods.
 *
 * @param mode The mode character.
 * @param enable true to set the mode; false to remove it.
 * @param param Additional parameter for the mode (if required).
 */
void Channel::setMode(char mode, bool enable, const std::string& param)
{
    _modes[mode] = enable;
    switch (mode)
    {
        case 'i':
            _inviteOnly = enable;
            break;
        case 't':
            _topicRestricted = enable;
            break;
        case 'k':
            if (enable && !param.empty())
                _channelKey = param;
            else if (!enable)
                _channelKey = "";
            break;
        case 'l':
            if (enable && !param.empty())
            {
                int limit = std::stoi(param);
                if (limit <= 0)
                    throw std::invalid_argument("User limit must be positive");
                _userLimit = limit;
            }
            else if (!enable)
                _userLimit = 0;
            break;
        case 'o':
            break;
        default:
            break;
    }
}


/**
 * @brief Checks if a specific mode is set for the channel.
 *
 * Looks up the mode flag in the _modes map.
 *
 * @param mode The mode character to check.
 * @return true if the mode is enabled; false otherwise.
 */
bool Channel::hasMode(char mode) const
{
    auto it = _modes.find(mode);
    if (it != _modes.end()) return it->second;
    return false;
}


/**
 * @brief Retrieves the list of client file descriptors in the channel.
 *
 * @return A constant reference to the vector containing file descriptors of
 * channel members.
 */
const std::vector<int>& Channel::getClients() const
{
    return _clients;
}


/**
 * @brief Adds an operator to the channel.
 *
 * If the specified client is not already an operator, adds its file descriptor
 * to the _operators vector.
 *
 * @param fd The file descriptor of the client to be granted operator status.
 */
void Channel::addOperator(int fd)
{
    // If the user is already an operator, do nothing.
    if (!isOperator(fd)) _operators.push_back(fd);
}


/**
 * @brief Removes an operator from the channel.
 *
 * Searches for the client's file descriptor in the _operators vector and
 * removes it.
 *
 * @param fd The file descriptor of the client whose operator status should be
 * removed.
 */
void Channel::removeOperator(int fd)
{
    for (auto it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (*it == fd)
        {
            _operators.erase(it);
            break;
        }
    }
}


/**
 * @brief Checks if a client is an operator in the channel.
 *
 * Iterates over the _operators vector to determine if the given file descriptor
 * corresponds to an operator.
 *
 * @param fd The file descriptor of the client.
 * @return true if the client is an operator; false otherwise.
 */
bool Channel::isOperator(int fd) const
{
    for (int op : _operators)
    {
        if (op == fd) return true;
    }
    return false;
}


/**
 * @brief Invites a client to the channel.
 *
 * Adds the client's file descriptor to the `_invitedClients` set, allowing them to join
 * the channel even if it is invite-only.
 *
 * @param fd The file descriptor of the client to be invited.
 */
void Channel::inviteClient(int fd)
{
    _invitedClients.insert(fd);
}


/**
 * @brief Checks if a client has been invited to the channel.
 *
 * Determines whether the given file descriptor exists in the `_invitedClients` set.
 *
 * @param fd The file descriptor of the client.
 * @return `true` if the client has been invited; `false` otherwise.
 */
bool Channel::isInvited(int fd) const
{
    return _invitedClients.find(fd) != _invitedClients.end();
}


/**
 * @brief Removes an invite for a client.
 *
 * If a client was invited but has not yet joined, this function revokes their invitation.
 *
 * @param fd The file descriptor of the client whose invite should be removed.
 */
void Channel::removeInvite(int fd)
{
    _invitedClients.erase(fd);
}