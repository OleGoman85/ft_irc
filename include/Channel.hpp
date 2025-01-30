/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alisa <alisa@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:47:36 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/30 16:34:45 by alisa            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <bitset>

/**
 * @brief Represents an IRC channel.
 *
 * The Channel class encapsulates the state and functionality associated
 * with an IRC channel, including its name, topic, members, modes, and operator list.
 */
class Channel {
public:
    enum ModeFlags { INVITE_ONLY = 0, TOPIC_RESTRICTED, PASSWORD_PROTECTED, USER_LIMIT, OPERATORS };

    /**
     * @brief Constructs a new Channel object.
     *
     * Initializes the channel with the given name and default settings.
     *
     * @param name The name of the channel.
     */
    Channel(const std::string& name);
    ~Channel();

    std::string getName() const;
    void addClient(int fd);
    void removeClient(int fd);
    bool hasClient(int fd) const;
    
    void setTopic(const std::string& topic);
    std::string getTopic() const;
    
    void setMode(char mode, bool enable, const std::string& param = "");
    bool hasMode(char mode) const;

    const std::vector<int>& getClients() const;

    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd) const;

    bool isInviteOnly() const { return _modes.test(INVITE_ONLY); }
    bool isTopicRestricted() const { return _modes.test(TOPIC_RESTRICTED); }

    const std::string& getChannelKey() const { return _channelKey; }
    int getUserLimit() const { return _userLimit; }

private:
    std::string _name;
    std::vector<int> _clients;
    std::string _topic;
    std::bitset<8> _modes;  // for storing channel modes

    // bool _inviteOnly;
    // bool _topicRestricted;
    std::string _channelKey;
    std::vector<int> _operators;
    int _userLimit;
};

#endif // CHANNEL_HPP
