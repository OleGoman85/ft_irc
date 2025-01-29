/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:47:36 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/20 07:41:40 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>

/**
 * @brief Represents an IRC channel.
 *
 * The Channel class encapsulates the state and functionality associated
 * with an IRC channel, including its name, topic, members, modes, and operator list.
 */
class Channel {
public:
    /**
     * @brief Constructs a new Channel object.
     *
     * Initializes the channel with the given name and default settings.
     * The channel topic is set to an empty string, invite-only and topic-restricted modes are disabled,
     * the channel key is empty, and the user limit is set to 0 (meaning no limit).
     *
     * @param name The name of the channel.
     */
    Channel(const std::string& name);
    ~Channel();


    /**
     * @brief Retrieves the channel name.
     *
     * @return The name of the channel.
     */
    std::string getName() const;


    /**
     * @brief Adds a client to the channel.
     *
     * Adds the client (represented by its file descriptor) to the channel if the client is not already a member.
     *
     * @param fd The file descriptor of the client to add.
     */
    void addClient(int fd);


    /**
     * @brief Removes a client from the channel.
     *
     * Removes the client (by file descriptor) from the channel's member list.
     *
     * @param fd The file descriptor of the client to remove.
     */
    void removeClient(int fd);


    /**
     * @brief Checks if a client is a member of the channel.
     *
     * @param fd The file descriptor of the client.
     * @return true if the client is in the channel; false otherwise.
     */
    bool hasClient(int fd) const;
    
    
    /**
     * @brief Sets the channel topic.
     *
     * Updates the channel's topic to the provided string.
     *
     * @param topic The new topic for the channel.
     */
    void setTopic(const std::string& topic);


    /**
     * @brief Retrieves the current channel topic.
     *
     * @return The current topic of the channel.
     */
    std::string getTopic() const;
    
    
    /**
     * @brief Sets a mode for the channel.
     *
     * Updates a specific mode flag for the channel, optionally using an additional parameter.
     * Supported modes include:
     * - 'i' : Invite-only mode.
     * - 't' : Topic restricted mode (only operators can change the topic).
     * - 'k' : Channel key (password). When enabled, the parameter should contain the key.
     * - 'l' : User limit. When enabled, the parameter should be a valid numeric string.
     * - 'o' : Operator mode. Note: Operator management is performed via separate methods.
     *
     * @param mode The mode character.
     * @param enable true to activate the mode, false to deactivate.
     * @param param An optional parameter required for certain modes (default is an empty string).
     */
    void setMode(char mode, bool enable, const std::string& param = "");


    /**
     * @brief Checks if a specific mode is active on the channel.
     *
     * @param mode The mode character to check.
     * @return true if the mode is active; false otherwise.
     */
    bool hasMode(char mode) const;


    /**
     * @brief Retrieves the list of client file descriptors in the channel.
     *
     * @return A constant reference to the vector of client file descriptors.
     */
    const std::vector<int>& getClients() const;


    /**
     * @brief Adds an operator to the channel.
     *
     * Grants operator status to the client with the given file descriptor.
     * If the client is already an operator, no action is taken.
     *
     * @param fd The file descriptor of the client to be granted operator privileges.
     */
    void addOperator(int fd);


    /**
     * @brief Removes an operator from the channel.
     *
     * Revokes operator status from the client with the given file descriptor.
     *
     * @param fd The file descriptor of the client whose operator privileges are to be removed.
     */
    void removeOperator(int fd);


    /**
     * @brief Checks if a client is an operator in the channel.
     *
     * @param fd The file descriptor of the client.
     * @return true if the client is an operator; false otherwise.
     */
    bool isOperator(int fd) const;


    /**
     * @brief Checks if the channel is in invite-only mode.
     *
     * @return true if invite-only mode is enabled; false otherwise.
     */
    bool isInviteOnly() const { return _inviteOnly; }
    
    
    /**
     * @brief Checks if the channel has topic restrictions.
     *
     * @return true if topic changes are restricted to operators; false otherwise.
     */
    bool isTopicRestricted() const { return _topicRestricted; }
    
    
    /**
     * @brief Retrieves the channel key.
     *
     * @return A constant reference to the channel key (password).
     */
    const std::string& getChannelKey() const { return _channelKey; }
    
    
    /**
     * @brief Retrieves the user limit for the channel.
     *
     * @return The maximum number of users allowed in the channel (0 if no limit).
     */
    int getUserLimit() const { return _userLimit; }

private:
    std::string _name;             ///< The name of the channel.
    std::vector<int> _clients;     ///< List of client file descriptors that are members of the channel.
    std::string _topic;            ///< The topic of the channel.
    std::map<char, bool> _modes;   ///< A mapping of mode characters to their enabled/disabled state.

    // Additional fields for modes:
    bool _inviteOnly;              ///< Invite-only mode flag ('i').
    bool _topicRestricted;         ///< Topic restriction flag ('t').
    std::string _channelKey;       ///< Channel key (password) for mode 'k'.
    std::vector<int> _operators;   ///< List of file descriptors for clients with operator privileges ('o').
    int _userLimit;                ///< Maximum number of allowed users in the channel ('l', 0 means no limit).
};

#endif // CHANNEL_HPP
