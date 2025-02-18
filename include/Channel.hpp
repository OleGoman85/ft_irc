#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <map>
#include <set>
#include <string>
#include <vector>

/**
 * @brief Represents an IRC channel.
 *
 * The Channel class encapsulates the state and functionality associated
 * with an IRC channel, including its name, topic, members, modes, and operator
 * list.
 */
class Channel
{
public:
    Channel();

    /**
     * @brief Constructs a new Channel object.
     *
     * Initializes the channel with the given name and default settings.
     * - The channel topic is set to an empty string.
     * - Invite-only (`+i`) and topic-restricted (`+t`) modes are disabled.
     * - The channel key (`+k`) is empty.
     * - The user limit (`+l`) is set to `0` (no limit).
     *
     * @param name The name of the channel.
     */
    Channel(const std::string& name);
    
    ~Channel();

    /** @brief Retrieves the channel name. */
    std::string getName() const;

    /** @brief Adds a client to the channel if they are not already a member. */
    void addClient(int fd);

    /** @brief Removes a client from the channel. */
    void removeClient(int fd);

    /** @brief Checks if a client is a member of the channel. */
    bool hasClient(int fd) const;

    /** @brief Sets the channel topic. */
    void setTopic(const std::string& topic);

    /** @brief Retrieves the current channel topic. */
    std::string getTopic() const;

    /**
     * @brief Sets or removes a mode for the channel.
     *
     * Supported modes:
     * - 'i' : Invite-only mode (`+i` / `-i`).
     * - 't' : Topic restriction (`+t` / `-t`).
     * - 'k' : Channel key (password) (`+k <key>` / `-k`).
     * - 'l' : User limit (`+l <number>` / `-l`).
     * - 'o' : Operator status (handled separately).
     *
     * @param mode The mode character.
     * @param enable `true` to activate, `false` to deactivate.
     * @param param Optional parameter required for `+k` and `+l` modes.
     */
    void setMode(char mode, bool enable, const std::string& param = "");

    /** @brief Checks if a specific mode is active on the channel. */
    bool hasMode(char mode) const;

    /** @brief Retrieves the list of clients in the channel. */
    const std::vector<int>& getClients() const;

    /** @brief Grants operator status to a client. */
    void addOperator(int fd);

    /** @brief Revokes operator status from a client. */
    void removeOperator(int fd);

    /** @brief Checks if a client is an operator in the channel. */
    bool isOperator(int fd) const;

    /** @brief Checks if the channel is in invite-only mode (`+i`). */
    bool isInviteOnly() const { return _inviteOnly; }

    /** @brief Checks if topic changes are restricted to operators (`+t`). */
    bool isTopicRestricted() const { return _topicRestricted; }

    /** @brief Retrieves the channel key (`+k`). */
    const std::string& getChannelKey() const { return _channelKey; }

    /** @brief Retrieves the user limit (`+l`). Returns `0` if no limit is set. */
    int getUserLimit() const { return _userLimit; }

    /** @brief Invites a client to the channel. */
    void inviteClient(int fd);

    /** @brief Checks if a client has been invited. */
    bool isInvited(int fd) const;

    /** @brief Removes an invite for a client. */
    void removeInvite(int fd);

private:
    std::string _name;                 ///< Channel name.
    std::vector<int> _clients;          ///< List of clients in the channel.
    std::string _topic;                 ///< Channel topic.
    std::map<char, bool> _modes;        ///< Map of active channel modes.

    // Additional fields for specific modes:
    bool _inviteOnly;                    ///< Invite-only mode (`+i`).
    bool _topicRestricted;                ///< Topic restriction mode (`+t`).
    std::string _channelKey;              ///< Channel key for mode `+k`.
    std::vector<int> _operators;          ///< List of operator client FDs (`+o`).
    int _userLimit;                       ///< User limit for mode `+l`. `0` means no limit.

    std::set<int> _invitedClients;        ///< Set of invited clients.
};

#endif  // CHANNEL_HPP
