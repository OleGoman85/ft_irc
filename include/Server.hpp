#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <sys/socket.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"
#include "FileTransfer.hpp"

/**
 * @brief Represents an IRC server.
 *
 * The `Server` class manages the core functionality of an IRC server, including:
 * - Managing client connections and handling authentication.
 * - Handling message routing between users and channels.
 * - Running the main event loop using `poll()` for non-blocking I/O.
 * - Processing commands received from clients.
 */
class Server
{
public:
    /**
     * @brief Constructs a new Server object.
     *
     * Initializes the server with the provided port and password.
     * Sets up internal data structures and configures the listening socket.
     *
     * @param port The port number on which the server listens for incoming connections.
     * @param password The connection password required by clients.
     */
    Server(int port, const std::string& password);
    

    /** @brief Destructor for the Server class. Cleans up resources. */
    ~Server();


    /**
     * @brief Runs the main server loop.
     *
     * Starts the event loop, which continuously:
     * - Polls for incoming data and new connections.
     * - Reads messages from clients and processes them.
     * - Sends responses back to clients when necessary.
     */
    void run();


    /**
     * @brief Removes a client from the server.
     *
     * - Closes the client socket.
     * - Removes the client from the internal client map.
     * - Removes the client’s poll descriptor.
     * - Notifies relevant channels of the disconnection.
     *
     * @param fd The file descriptor of the client to be removed.
     */
    void removeClient(int fd);


    /**
     * @brief Broadcasts a message to all connected clients except the sender.
     *
     * Iterates over all connected clients and sends the message to each one, 
     * except for the client with the specified sender file descriptor.
     *
     * @param message The message to be broadcasted.
     * @param sender_fd The file descriptor of the sender (who will not receive the message).
     */
    void broadcastMessage(const std::string& message, int sender_fd);


    /**
     * @brief Retrieves the current server password.
     *
     * @return A constant reference to the server password.
     */
    const std::string& getPassword() const;


    /**
     * @brief Changes the server password.
     *
     * @param newPassword The new password to be set.
     */
    void setPassword(const std::string& newPassword);


    /**
     * @brief Retrieves the map of all connected clients.
     *
     * @return A reference to the internal map of clients.
     */
    std::map<int, std::unique_ptr<Client>>& getClients();


    /**
     * @brief Retrieves the map of all channels.
     *
     * @return A reference to the internal map of channels.
     */
    std::map<std::string, Channel>& getChannels();


    /**
     * @brief Retrieves the map of all ongoing file transfers.
     *
     * @return A reference to the internal map of file transfers.
     */
    std::map<std::string, FileTransfer>& getFileTransfers();


    /**
     * @brief Retrieves the server name.
     *
     * @return A constant reference to the server name.
     */
    const std::string& getServerName() const;
    

    /**
     * @brief Sends a message to a client, ensuring that it is properly buffered if the socket is not immediately writable.
     *
     * @param fd The file descriptor of the client.
     * @param message The message to be sent.
     */
    void safeSend(int fd, const std::string& message);


    /**
     * @brief Sends an error message indicating that a password is required.
     *
     * @param fd The file descriptor of the client.
     */
    void passRequired(int fd);


    /**
     * @brief Sends an error message indicating that the client may not be registered.
     *
     * @param fd The file descriptor of the client.
     */
    void mayNotRegistered(int fd);


    /**
     * @brief Sends an error message indicating that the client is not registered.
     *
     * @param fd The file descriptor of the client.
     */
    void notRegistered(int fd);


private:
    int _port;       ///< The port number on which the server listens.
    int _listen_fd;  ///< The listening socket file descriptor.

    std::vector<struct pollfd> _poll_fds;  ///< List of poll descriptors (server + clients).

    std::string _password;  ///< Server connection password.
    
    std::map<int, std::unique_ptr<Client>> _clients;  ///< Active clients.
    std::map<std::string, Channel> _channels;  ///< Active channels.
    std::map<std::string, FileTransfer> _fileTransfers; ///< Ongoing file transfers.

    std::string _serverName; ///< The name of the IRC server.

    /**
     * @brief Sets up the server's listening socket.
     *
     * - Creates a non-blocking socket.
     * - Binds the socket to the specified port.
     * - Begins listening for incoming connections.
     *
     * @throws std::runtime_error if any socket operations fail.
     */
    void setupServer();


    /**
     * @brief Accepts a new client connection.
     *
     * - Accepts a connection from the listening socket.
     * - Sets the client socket to non-blocking mode.
     * - Adds the client to the list of tracked connections.
     */
    void acceptNewConnection();


    /**
     * @brief Handles incoming data from a client.
     *
     * - Reads available data from the client socket.
     * - Buffers the data for message processing.
     * - Extracts and processes complete commands from the input buffer.
     *
     * @param fd The file descriptor of the client.
     */
    void handleClientData(int fd);


    /**
     * @brief Processes a complete command received from a client.
     *
     * - Parses the command into tokens.
     * - Dispatches the command to the appropriate handler function.
     *
     * @param fd The file descriptor of the client that sent the command.
     * @param command The complete command string received.
     */
    void processCommand(int fd, const std::string& command);
};

#endif  // SERVER_HPP
