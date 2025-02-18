#include "../include/Client.hpp"

/**
 * @brief Constructs a Client object.
 *
 * Initializes the client with a given socket file descriptor (`fd`).
 * Sets default values for authentication state, nickname, username, host, 
 * real name, and input/output buffers.
 *
 * @param fd The socket file descriptor associated with the client.
 */
Client::Client(int fd)
    : outBuffer(""),  ///< Initializes the outgoing message buffer as empty.
      buffer(""),     ///< Initializes the incoming data buffer as empty.
      authState(NOT_REGISTERED),  ///< Sets initial authentication state to NOT_REGISTERED.
      _fd(fd),        ///< Assigns the socket file descriptor.
      _nickname(""),  ///< Initializes the nickname as an empty string.
      _username(""),  ///< Initializes the username as an empty string.
      _host("localhost"),  ///< Defaults the host to "localhost".
      _realName("")   ///< Initializes the real name as an empty string.
{
}

/**
 * @brief Destroys the Client object.
 *
 * This destructor does not require specific cleanup, as memory management 
 * is handled externally.
 */
Client::~Client()
{
}

/**
 * @brief Retrieves the client's socket file descriptor.
 *
 * @return The file descriptor associated with the client's socket.
 */
int Client::getFd() const
{
    return _fd;
}

/**
 * @brief Retrieves the client's nickname.
 *
 * @return The current nickname of the client.
 */
std::string Client::getNickname() const
{
    return _nickname;
}

/**
 * @brief Sets the client's nickname.
 *
 * Updates the client's nickname to the provided value.
 *
 * @param newNickname The new nickname to assign.
 */
void Client::setNickname(const std::string& newNickname)
{
    _nickname = newNickname;
}

/**
 * @brief Retrieves the client's username.
 *
 * @return The current username of the client.
 */
std::string Client::getUsername() const
{
    return _username;
}

/**
 * @brief Sets the client's username.
 *
 * Updates the username to the given value.
 *
 * @param newUsername The new username to assign.
 */
void Client::setUsername(const std::string& newUsername)
{
    _username = newUsername;
}

/**
 * @brief Retrieves the client's host.
 *
 * @return The host address of the client.
 */
std::string Client::getHost() const
{
    return _host;
}

/**
 * @brief Sets the client's host.
 *
 * Updates the host field with the specified value.
 *
 * @param newHost The new host address to assign.
 */
void Client::setHost(const std::string& newHost)
{
    _host = newHost;
}

/**
 * @brief Retrieves the client's real name.
 *
 * @return The real name of the client.
 */
std::string Client::getRealName() const 
{
    return _realName;
}

/**
 * @brief Sets the client's real name.
 *
 * Updates the real name of the client.
 *
 * @param realName The new real name to assign.
 */
void Client::setRealName(const std::string& realName) 
{
    _realName = realName;
}
