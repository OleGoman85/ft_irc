/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:46:57 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/16 12:08:12 by ogoman           ###   ########.fr       */
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
Client::Client(int fd)
    : _fd(fd),             // Initialize the file descriptor first as declared.
      nickname(""),        // Initialize nickname as an empty string.
      username(""),        // Initialize username as an empty string.
      buffer(""),          // Initialize the incoming data buffer as empty.
      authState(NOT_REGISTERED) {}// Set initial authentication state to NOT_REGISTERED.


Client::~Client() {}

int Client::getFd() const {return _fd;}


//!
/*
Задача:

Представлять отдельного клиента.
Хранить информацию о клиенте (ник, имя пользователя, буфер данных).
*/
