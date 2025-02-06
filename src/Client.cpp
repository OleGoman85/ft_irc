/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:46:57 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/05 09:41:49 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

/**
 * @brief Client constructor.
 *
 * Initializes a new Client object with the given file descriptor.
 * The constructor initializes the client's socket descriptor, and sets the initial
 * values for nickname, username, buffer, and authentication state.
 * Initially, the client is marked as NOT_REGISTERED.
 *
 * @param fd The file descriptor for the client's connection.
 */
Client::Client(int fd) :
      buffer(""),          // Initialize the incoming data buffer as empty.
      authState(NOT_REGISTERED), // Set initial authentication state to NOT_REGISTERED.
      _fd(fd),             // Initialize the file descriptor first as declared.
      _nickname(""),        // Initialize nickname as an empty string.
      _username("")        // Initialize username as an empty string.
      {}


Client::~Client() {}

int Client::getFd() const {return _fd;}

std::string Client::getNickname() const {return _nickname;}

void Client::setNickname(const std::string& newNickname) {_nickname = newNickname;}

std::string Client::getUsername() const {return _username;}

void Client::setUsername(const std::string& newUsername) {_username = newUsername;}


//!
/*
Задача:

Представлять отдельного клиента.
Хранить информацию о клиенте (ник, имя пользователя, буфер данных).
*/
