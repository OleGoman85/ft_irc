#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

/**
 * @brief Enum representing the authentication state of a client.
 *
 * This enum defines the various stages of client registration in the IRC server.
 */
enum AuthState
{
    NOT_REGISTERED,    ///< The client has not yet sent the required registration commands.
    WAITING_FOR_NICK,  ///< The client has provided a PASS but has not yet set a nickname.
    WAITING_FOR_USER,  ///< The client has set a nickname but has not yet provided the USER command.
    AUTH_REGISTERED    ///< The client has completed registration and is fully connected.
};

/**
 * @brief Represents a connected IRC client.
 *
 * The `Client` class holds essential information about a connected IRC client,
 * including:
 * - Socket file descriptor (`_fd`)
 * - Nickname and username
 * - Host information
 * - Input/output buffers for handling messages
 * - Authentication state
 */
class Client
{
public:
    /**
     * @brief Constructs a Client object with the given file descriptor.
     *
     * @param fd The file descriptor of the client's socket.
     */
    Client(int fd);

    /** @brief Destructor for the Client class. */
    ~Client();

    /** @brief Retrieves the client's socket file descriptor. */
    int getFd() const;

    /** @brief Retrieves the client's current nickname. */
    std::string getNickname() const;

    /** @brief Sets a new nickname for the client. */
    void setNickname(const std::string& newNickname);

    /** @brief Retrieves the client's username. */
    std::string getUsername() const;

    /** @brief Sets the client's username. */
    void setUsername(const std::string& newUsername);

    /** @brief Retrieves the client's host address. */
    std::string getHost() const;

    /** @brief Sets the client's host address. */
    void setHost(const std::string& newHost);

    /** @brief Retrieves the client's real name. */
    std::string getRealName() const;

    /** @brief Sets the client's real name. */
    void setRealName(const std::string& realName);

    std::string outBuffer;   ///< Buffer storing unsent outgoing messages.
    std::string buffer;      ///< Buffer for storing incoming messages.
    AuthState   authState;   ///< Current authentication state of the client.

private:
    int         _fd;        ///< File descriptor for the client socket.
    std::string _nickname;  ///< Client's nickname.
    std::string _username;  ///< Client's username.
    std::string _host;      ///< Client's host address.
    std::string _realName;  ///< Client's real name (set via USER command).
};

#endif  // CLIENT_HPP
