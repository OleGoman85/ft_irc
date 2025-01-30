/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alisa <alisa@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:48:05 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/30 16:35:45 by alisa            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"
#include <cstdlib> // for std::stoi

Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
    //   _inviteOnly(false),
    //   _topicRestricted(false),
      _channelKey(""),
      _userLimit(0)
{
    _modes.reset(); // all modes are not active initially
}

Channel::~Channel() {}

std::string Channel::getName() const {
    return _name;
}

void Channel::addClient(int fd) {
    if (!hasClient(fd))
        _clients.push_back(fd);
}

void Channel::removeClient(int fd) {
    _clients.erase(std::remove(_clients.begin(), _clients.end(), fd), _clients.end());
}

bool Channel::hasClient(int fd) const {
    return std::find(_clients.begin(), _clients.end(), fd) != _clients.end();
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

std::string Channel::getTopic() const {
    return _topic;
}

void Channel::setMode(char mode, bool enable, const std::string& param) {
    switch (mode) {
        case 'i':
            _modes.set(INVITE_ONLY, enable);
            break;
        case 't':
            _modes.set(TOPIC_RESTRICTED, enable);
            break;
        case 'k':
            _modes.set(PASSWORD_PROTECTED, enable);
            if (enable && !param.empty())
                _channelKey = param;
            else
                _channelKey = "";
            break;
        case 'l':
            _modes.set(USER_LIMIT, enable);
            if (enable && !param.empty())
                _userLimit = std::stoi(param);
            else
                _userLimit = 0;
            break;
        case 'o':
            _modes.set(OPERATORS, enable);
            break;
    }
}

bool Channel::hasMode(char mode) const {
    switch (mode) {
        case 'i': return _modes.test(INVITE_ONLY);
        case 't': return _modes.test(TOPIC_RESTRICTED);
        case 'k': return _modes.test(PASSWORD_PROTECTED);
        case 'l': return _modes.test(USER_LIMIT);
        case 'o': return _modes.test(OPERATORS);
    }
    return false;
}

const std::vector<int>& Channel::getClients() const {
    return _clients;
}

void Channel::addOperator(int fd) {
    if (!isOperator(fd))
        _operators.push_back(fd);
}

void Channel::removeOperator(int fd) {
    _operators.erase(std::remove(_operators.begin(), _operators.end(), fd), _operators.end());
}

bool Channel::isOperator(int fd) const {
    return std::find(_operators.begin(), _operators.end(), fd) != _operators.end();
}
