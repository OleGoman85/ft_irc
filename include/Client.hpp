/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:46:13 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/12 10:02:47 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

/**
 * @brief Enum representing the authentication state of a client.
 *
 * This enum defines the various stages of client registration.
 */
enum AuthState
{
    NOT_REGISTERED,    ///< The client has not yet registered.
    WAITING_FOR_NICK,  ///< The client has passed the PASS command and is
                       ///< waiting to set a nickname.
    WAITING_FOR_USER,  ///< The client has set a nickname and is waiting for
                       ///< USER command.
    AUTH_REGISTERED    ///< The client is fully registered.
};

/**
 * @brief Represents a connected IRC client.
 *
 * The Client class holds information about a connected client,
 * such as its socket file descriptor, nickname, username, input buffer,
 * and authentication state.
 */
class Client
{
public:
    Client(int fd);
    ~Client();

    /**
     * @brief Retrieves the client's socket file descriptor.
     *
     * @return The file descriptor associated with the client's socket.
     */
    int getFd() const;

    std::string getNickname() const;
    void        setNickname(const std::string& newNickname);

    std::string getUsername() const;
    void        setUsername(const std::string& newUsername);

    std::string getHost() const;
    void setHost(const std::string& newHost);

    std::string getRealName() const;
    void        setRealName(const std::string& realName);

    std::string buffer;     ///< Buffer for storing incoming data.
    AuthState   authState;  ///< Current authentication state of the client.

private:
    int         _fd;        ///< File descriptor for the client socket.
    std::string _nickname;  ///< Client's nickname
    std::string _username;  ///< Client's username
    std::string _host;
    std::string _realName;
};

#endif  // CLIENT_HPP
