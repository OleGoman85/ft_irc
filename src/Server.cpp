#include "../include/Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "../commands/BotCommand.hpp"
#include "../commands/Cap.hpp"
#include "../commands/FileCommand.hpp"
#include "../commands/Invite.hpp"
#include "../commands/Join.hpp"
#include "../commands/Kick.hpp"
#include "../commands/List.hpp"
#include "../commands/Mode.hpp"
#include "../commands/Nick.hpp"
#include "../commands/Part.hpp"
#include "../commands/Pass.hpp"
#include "../commands/Privmsg.hpp"
#include "../commands/Quit.hpp"
#include "../commands/Topic.hpp"
#include "../commands/User.hpp"
#include "../commands/Who.hpp"
#include "../commands/Whois.hpp"
#include "../include/Utils.hpp"

/**
 * @brief Flushes the output buffer for a client.
 *
 * This function attempts to send all pending data stored in the client's
 * `outBuffer`. It repeatedly calls `send()` until:
 * - The buffer is empty (all data has been sent).
 * - The socket is not ready for writing (`EAGAIN` / `EWOULDBLOCK`).
 * - A critical error occurs, in which case the client is removed from the server.
 *
 * If `send()` encounters a non-recoverable error, the function calls `removeClient()`
 * to disconnect the client.
 *
 * @param server Pointer to the Server instance.
 * @param fd The file descriptor of the client whose output buffer is to be flushed.
 */
static void flushClientOutBuffer(Server* server, int fd)
{
    // Retrieve the map of connected clients.
    auto& clients = server->getClients();

    // Ensure the client exists before proceeding.
    if (clients.find(fd) == clients.end())
        return;  // Client not found, nothing to flush.

    // Get a pointer to the client.
    Client* client = clients[fd].get();

    // Attempt to send data until the buffer is empty or the socket is not writable.
    while (!client->outBuffer.empty())
    {
        // Send as much data as possible from the buffer.
        ssize_t sent = send(fd, client->outBuffer.c_str(), client->outBuffer.size(), 0);

        if (sent < 0)
        {
            // If the socket is temporarily unavailable, exit and retry later.
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                // If a serious error occurs, remove the client from the server.
                server->removeClient(fd);
                return;
            }
        }

        // Remove the sent portion from the outBuffer.
        client->outBuffer.erase(0, sent);
    }
}


/**
 * @brief Sends data to a client safely.
 *
 * This function ensures reliable data transmission to the client by:
 * 1. Attempting to flush any pending data stored in the client's `outBuffer`.
 * 2. Sending the new message immediately, if possible.
 * 3. If the socket cannot send all data (or is not ready for writing), 
 *    the unsent portion is appended to the client's `outBuffer` to be sent later.
 * 
 * This prevents data loss and ensures that messages are delivered in order, even
 * if the client’s socket is temporarily unable to receive data.
 *
 * @param fd The file descriptor of the client to which the message is sent.
 * @param message The message to send.
 */
void Server::safeSend(int fd, const std::string& message)
{
    // Retrieve the map of connected clients.
    auto& clients = getClients();
    
    // Ensure the client exists before attempting to send data.
    if (clients.find(fd) == clients.end()) return;  // Client not found, no action needed.

    // Get a pointer to the client.
    Client* client = clients[fd].get();

    // Flush any previously buffered data to avoid excessive memory growth.
    flushClientOutBuffer(this, fd);

    // Attempt to send the new message immediately.
    ssize_t sent = send(fd, message.c_str(), message.size(), 0);
    
    if (sent < 0)
    {
        // If the socket is temporarily unavailable, buffer the entire message for later.
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            client->outBuffer += message;
        }
        else
        {
            // For any other error, remove the client from the server.
            removeClient(fd);
        }
    }
    else if (static_cast<size_t>(sent) < message.size())
    {
        // If only part of the message was sent, store the remaining part in the buffer.
        client->outBuffer += message.substr(sent);
    }
}


/**
 * @brief Server constructor.
 *
 * Initializes the server with the given port and password.
 * This constructor:
 * - Sets the port number and password.
 * - Initializes internal data structures, including maps for clients and channels.
 * - Calls `setupServer()` to configure the listening socket and prepare for incoming connections.
 *
 * @param port The port number on which the server listens for client connections.
 * @param password The password required for clients to connect (if applicable).
 */
Server::Server(int port, const std::string& password)
    : _port(port),        // Assign the specified port for the server.
      _listen_fd(-1),     // Initialize the listening socket file descriptor as invalid.
      _poll_fds(),        // Initialize the poll descriptor list for handling multiple clients.
      _password(password),// Store the connection password.
      _clients(),         // Initialize the map to manage connected clients.
      _channels(),        // Initialize the map to store active IRC channels.
      _serverName("AwesomeIRC") // Set the server's name (can be modified if needed).
{
    setupServer(); // Configure the listening socket and prepare for incoming connections.
}


/**
 * @brief Server destructor.
 *
 * Closes the listening socket if it is open.
 */
Server::~Server()
{
    if (_listen_fd != -1) close(_listen_fd);
}


/**
 * @brief Sets up the server socket.
 *
 * This function:
 * - Creates a non-blocking TCP socket.
 * - Configures socket options to allow address reuse and disable Nagle's algorithm.
 * - Binds the socket to the specified port.
 * - Starts listening for incoming connections.
 * - Adds the listening socket to the poll descriptor vector for event monitoring.
 *
 * @throws std::runtime_error if any socket operation fails.
 */
void Server::setupServer()
{
    // Create a TCP socket (IPv4, Stream-based)
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_fd < 0)
        throw std::runtime_error("Failed to create socket");

    // Enable SO_REUSEADDR to allow quick reuse of the port after server restart
    int opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt SO_REUSEADDR failed");

    // Disable Nagle's algorithm (TCP_NODELAY) to reduce latency for small packets
    int flag = 1;
    if (setsockopt(_listen_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0)
        throw std::runtime_error("setsockopt TCP_NODELAY failed");

    // Set the socket to non-blocking mode to avoid blocking on accept() calls
    if (fcntl(_listen_fd, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to set non-blocking mode");

    // Configure the server's address structure (IPv4)
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr)); // Zero out the structure
    addr.sin_family      = AF_INET;      // IPv4
    addr.sin_addr.s_addr = INADDR_ANY;   // Accept connections on any network interface
    addr.sin_port        = htons(_port); // Convert port to network byte order

    // Bind the socket to the specified address and port
    if (bind(_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind failed");

    // Start listening for incoming connections (SOMAXCONN sets the maximum queue size)
    if (listen(_listen_fd, SOMAXCONN) < 0)
        throw std::runtime_error("listen failed");

    // Add the listening socket to the poll descriptor list for event monitoring
    struct pollfd pfd;
    pfd.fd      = _listen_fd;
    pfd.events  = POLLIN; // Monitor for incoming connections
    pfd.revents = 0;
    _poll_fds.push_back(pfd);

    std::cout << "Server started on port " << _port << "\n";
}



/**
 * @brief Runs the main server loop.
 *
 * This function continuously monitors all active file descriptors using `poll()`
 * to handle incoming connections, process client data, and send pending messages.
 * It ensures smooth operation by:
 * - Updating the poll event flags based on each client's state.
 * - Accepting new connections when the listening socket is ready.
 * - Reading incoming data from active clients.
 * - Sending pending messages if a client's socket is ready for writing.
 */
void Server::run()
{
    // Main event loop – runs indefinitely while the server is active.
    while (true)
    {
        // Update poll events for each client socket (skip the listening socket at index 0).
        // If a client's `outBuffer` is non-empty, monitor both readability (POLLIN) and writability (POLLOUT).
        for (size_t i = 1; i < _poll_fds.size(); ++i)
        {
            int   fd      = _poll_fds[i].fd;
            auto& clients = getClients();

            // Ensure the client still exists before updating poll flags.
            if (clients.find(fd) != clients.end())
            {
                Client* client = clients[fd].get();

                // If there is no pending output data, only check for incoming data (POLLIN).
                // Otherwise, also check if the socket is ready to send data (POLLOUT).
                _poll_fds[i].events = client->outBuffer.empty() ? POLLIN : (POLLIN | POLLOUT);
            }
        }

        // Monitor all file descriptors using `poll()` with a timeout of 100 milliseconds.
        int poll_count = poll(_poll_fds.data(), _poll_fds.size(), 100);
        if (poll_count < 0)
        {
            std::cerr << "poll error\n";
            continue;  // Log the error and continue the loop.
        }

        // Process events for each file descriptor.
        for (size_t i = 0; i < _poll_fds.size(); ++i)
        {
            int fd = _poll_fds[i].fd;

            // If the socket is ready for writing (POLLOUT), flush any buffered data.
            if (_poll_fds[i].revents & POLLOUT)
            {
                flushClientOutBuffer(this, fd);
            }

            // If the socket is ready for reading (POLLIN):
            if (_poll_fds[i].revents & POLLIN)
            {
                // If the listening socket is ready, accept a new client connection.
                if (fd == _listen_fd)
                {
                    acceptNewConnection();
                }
                else
                {
                    // Otherwise, handle incoming data from an existing client.
                    handleClientData(fd);
                }
            }
        }
    }
}


/**
 * @brief Accepts a new client connection.
 *
 * This function handles new client connections by:
 * - Accepting the connection on the listening socket.
 * - Setting the client's socket to non-blocking mode.
 * - Disabling Nagle’s algorithm for improved responsiveness.
 * - Adding the client’s socket to the `poll` descriptor list.
 * - Creating a new `Client` object to manage the connection.
 * - Logging the client's IP address and port.
 *
 * If any step fails, the connection is closed to prevent resource leaks.
 */
void Server::acceptNewConnection()
{
    // Structure to store the client's address information
    struct sockaddr_in client_addr;
    socklen_t          client_len = sizeof(client_addr);

    // Accept a new connection from the listening socket
    int client_fd = accept(_listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0)
    {
        // If accept() fails due to a non-blocking socket, ignore and return.
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            std::cerr << "accept failed\n";
        return;
    }

    // Set the client's socket to non-blocking mode
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cerr << "Failed to set non-blocking mode for client\n";
        close(client_fd);  // Close the socket to prevent leaks
        return;
    }

    // Disable Nagle’s algorithm to reduce latency for real-time communication
    int flag = 1;
    if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0)
    {
        std::cerr << "setsockopt TCP_NODELAY failed for client\n";
        close(client_fd);
        return;
    }

    // Add the client’s socket to the poll descriptor list
    struct pollfd pfd;
    pfd.fd      = client_fd;   // Set file descriptor
    pfd.events  = POLLIN;      // Monitor for incoming data
    pfd.revents = 0;           // Initialize event status
    _poll_fds.push_back(pfd);  // Register with the polling system

    // Add the new client to the server's client list
    getClients().emplace(client_fd, std::make_unique<Client>(client_fd));

    // Log the successful connection with client IP and port
    std::cout << "New connection from "
              << inet_ntoa(client_addr.sin_addr)  // Convert IP to readable format
              << ":" << ntohs(client_addr.sin_port)  // Convert port to host byte order
              << " (fd: " << client_fd << ")\n";
}


/**
 * @brief Handles incoming data from a client.
 *
 * This function performs the following steps:
 * 1. Reads data from the client socket in a **non-blocking** manner.
 * 2. If data is received, it is appended to the client's input buffer.
 * 3. The function continuously checks for **complete commands** (terminated by "\r\n" or "\n").
 * 4. When a complete command is found:
 *    - It is **extracted, trimmed**, and passed to `processCommand()`.
 *    - The command is then removed from the buffer.
 * 5. If the client **disconnects** (recv returns 0), it is removed from the server.
 *
 * @param fd File descriptor of the client.
 */
void Server::handleClientData(int fd)
{
    char buffer[512];
    int  bytes_received = recv(fd, buffer, sizeof(buffer), 0);

    // If an error occurs while receiving data
    if (bytes_received < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "recv error on fd " << fd << "\n";
            removeClient(fd);  // Remove the client on critical errors
        }
        return;
    }

    // Log incoming data (useful for debugging)
    std::cout << "[INFO] Received from fd " << fd << ": "
              << std::string(buffer, bytes_received) << "\n";

    // Append received data to the client's input buffer
    getClients()[fd]->buffer.append(buffer, bytes_received);

    std::cout << "[INFO] Buffer for fd " << fd << ": \""
              << getClients()[fd]->buffer << "\"\n";

    size_t pos;
    
    // Process complete commands in the buffer
    while (true)
    {
        // Look for a complete command ending with "\r\n" or "\n"
        pos = getClients()[fd]->buffer.find("\r\n");
        if (pos == std::string::npos)
        {
            pos = getClients()[fd]->buffer.find("\n");
            if (pos != std::string::npos && pos > 0 &&
                getClients()[fd]->buffer[pos - 1] == '\r')
            {
                pos -= 1;
            }
        }

        // If no complete command is found, exit the loop
        if (pos == std::string::npos) break;

        // Extract the command from the buffer
        std::string command = getClients()[fd]->buffer.substr(0, pos);

        // Remove the processed command from the buffer
        if (getClients()[fd]->buffer.substr(pos, 2) == "\r\n")
            getClients()[fd]->buffer.erase(0, pos + 2);
        else
            getClients()[fd]->buffer.erase(0, pos + 1);

        // Trim whitespace from the command
        command.erase(0, command.find_first_not_of(" \t"));
        command.erase(command.find_last_not_of(" \t") + 1);

        // Log the extracted command
        std::cout << "[INFO] Processing command from fd " << fd << ": \""
                  << command << "\"\n";

        // Execute the command if it's not empty
        if (!command.empty()) processCommand(fd, command);

        // If the client was removed during command processing, stop further processing
        if (getClients().find(fd) == getClients().end()) return;
    }

    // Handle client disconnection
    if (bytes_received == 0)
    {
        std::cout << "Client (fd: " << fd << ") disconnected\n";
        removeClient(fd);
    }
}


/**
 * @brief Removes a client from the server and cleans up associated resources.
 *
 * This function ensures proper cleanup when a client disconnects or is forcibly removed.
 * It performs the following steps:
 *
 * 1. **Remove the client from all channels**:
 *    - Iterates over the server's channel list and removes the client from each.
 *    - If a channel becomes empty after removal, it is deleted from the `_channels` map.
 *
 * 2. **Close the client's socket** to free system resources.
 *
 * 3. **Remove the client from the server's client map** to ensure no stale references exist.
 *
 * 4. **Remove the client's file descriptor from the poll descriptor vector** to prevent unnecessary polling.
 *
 * @param fd File descriptor of the client to be removed.
 */
void Server::removeClient(int fd)
{
    // Iterate through all channels and remove the client from them
    for (std::map<std::string, Channel>::iterator it = _channels.begin();
         it != _channels.end(); ++it)
    {
        it->second.removeInvite(fd);
    }
    for (std::map<std::string, Channel>::iterator it = _channels.begin();
         it != _channels.end();)
    {
        it->second.removeClient(fd);  // Remove client from this channel

        // If the channel is now empty, erase it from the server
        if (it->second.getClients().empty())
        {
            std::map<std::string, Channel>::iterator eraseIt = it++;
            _channels.erase(eraseIt);
        }
        else
        {
            ++it;
        }
    }

    close(fd);

    // Remove the client from the server's client map
    getClients().erase(fd);
    // Remove the client's file descriptor from the poll descriptor vector
    for (size_t i = 0; i < _poll_fds.size(); ++i)
    {
        if (_poll_fds[i].fd == fd)
        {
            _poll_fds.erase(_poll_fds.begin() + i);
            break;
        }
    }
}



/**
 * @brief Broadcasts a message to all clients except the sender.
 *
 * This function iterates over all connected clients and sends the given message
 * to each client whose file descriptor is **not** equal to `sender_fd`.
 * 
 * It uses `safeSend()` instead of `send()` to ensure that messages are sent
 * correctly even if a client socket is not immediately writable.
 *
 * @param message The message to be broadcast.
 * @param sender_fd File descriptor of the sender (this client will be excluded
 * from receiving the message).
 */

// void Server::broadcastMessage(const std::string& message, int sender_fd)
// {
//     for (const auto& pair : getClients())
//     {
//         int client_fd = pair.first;
//         if (client_fd != sender_fd)
//             send(client_fd, message.c_str(), message.size(), 0);
//         fsync(client_fd);
//     }
// }

//!
void Server::broadcastMessage(const std::string& message, int sender_fd)
{
    for (const auto& pair : getClients())
    {
        int client_fd = pair.first;
        if (client_fd != sender_fd)
        {
            safeSend(client_fd, message);
        }
    }
}
//!


/**
 * @brief Processes a complete command received from a client.
 *
 * Splits the command string into tokens and dispatches it to the appropriate
 * command handler based on the first token.
 *
 * @param fd File descriptor of the client that sent the command.
 * @param command The complete command string.
 */
void Server::processCommand(int fd, const std::string& command)
{
    // std::cout << Utils::getTimestamp() << "Command from fd " << fd << ": " <<
    // command << std::endl;
    std::cout << "\033[1;32m" << Utils::getTimestamp() << "Command from fd "
              << fd << ": " << command << "\033[0m" << std::endl;
    std::vector<std::string> tokens = Utils::split(command, ' ');
    if (tokens.empty()) return;

    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    if (cmd == "PASS")
    {
        if (getClients()[fd]->authState != NOT_REGISTERED)
        {
            mayNotRegistered(fd);
            return;
        }
        handlePassCommand(this, fd, tokens, command);
    }
    else if (cmd == "NICK")
    {
        if (!_password.empty() && getClients()[fd]->authState == NOT_REGISTERED)
        {
            passRequired(fd);
            return;
        }
        handleNickCommand(this, fd, tokens, command);
    }
    else if (cmd == "USER")
    {
        if (!_password.empty() && getClients()[fd]->authState == NOT_REGISTERED)
        {
            passRequired(fd);
            return;
        }
        handleUserCommand(this, fd, tokens, command);
    }
    else if (cmd == "JOIN")
    {
        if (getClients()[fd]->authState != AUTH_REGISTERED)
        {
            notRegistered(fd);
            return;
        }
        handleJoinCommand(this, fd, tokens, command);
    }
    else if (cmd == "PRIVMSG")
    {
        if (getClients()[fd]->authState != AUTH_REGISTERED)
        {
            notRegistered(fd);
            return;
        }
        handlePrivmsgCommand(this, fd, tokens, command);
    }
    else if (cmd == "QUIT")
    {
        handleQuitCommand(this, fd, tokens, command);
    }
    else if (cmd == "PART")
    {
        if (getClients()[fd]->authState != AUTH_REGISTERED)
        {
            notRegistered(fd);
            return;
        }
        handlePartCommand(this, fd, tokens, command);
    }
    else if (cmd == "KICK")
    {
        if (getClients()[fd]->authState != AUTH_REGISTERED)
        {
            notRegistered(fd);
            return;
        }
        handleKickCommand(this, fd, tokens, command);
    }
    else if (cmd == "INVITE")
    {
        if (getClients()[fd]->authState != AUTH_REGISTERED)
        {
            notRegistered(fd);
            return;
        }
        handleInviteCommand(this, fd, tokens, command);
    }
    else if (cmd == "TOPIC")
    {
        if (getClients()[fd]->authState != AUTH_REGISTERED)
        {
            notRegistered(fd);
            return;
        }
        handleTopicCommand(this, fd, tokens, command);
    }
    else if (cmd == "MODE")
    {
        if (getClients()[fd]->authState != AUTH_REGISTERED)
        {
            notRegistered(fd);
            return;
        }
        handleModeCommand(this, fd, tokens, command);
    }
    else if (cmd == "FILE")
    {
        handleFileCommand(this, fd, tokens, command);
    }
    else if (cmd == "BOT")
    {
        handleBotCommand(this, fd, tokens, command);
    }
    else if (cmd == "PING")
    {
        std::string pong = "PONG ";
        if (tokens.size() > 1) pong += tokens[1];
        pong += "\r\n";
        send(fd, pong.c_str(), pong.size(), 0);
        std::cout << "Sending: " << pong;
    }
    else if (cmd == "WHO")
    {
        handleWhoCommand(this, fd, tokens, command);
    }
    else if (cmd == "WHOIS")
    {
        handleWhoisCommand(this, fd, tokens, command);
    }
    else if (cmd == "LIST")
    {
        handleListCommand(this, fd, tokens, command);
    }
    else if (cmd == "CAP")
    {
        handleCapCommand(this, fd, tokens, command);
    }

    else
    {
        std::string reply = "421 " + cmd + " :Unknown command\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
    }
}


/**
 * @brief Retrieves the server password.
 *
 * @return A constant reference to the server's password.
 */
const std::string& Server::getPassword() const
{
    return _password;
}


/**
 * @brief Updates the server password.
 *
 * @param newPassword The new password to be set.
 */
void Server::setPassword(const std::string& newPassword)
{
    _password = newPassword;
}


/**
 * @brief Retrieves the list of connected clients.
 *
 * @return A reference to the map of clients (key: file descriptor).
 */
std::map<int, std::unique_ptr<Client>>& Server::getClients()
{
    return _clients;
}


/**
 * @brief Retrieves the list of active channels.
 *
 * @return A reference to the map of channels (key: channel name).
 */
std::map<std::string, Channel>& Server::getChannels()
{
    return _channels;
}


/**
 * @brief Retrieves the list of ongoing file transfers.
 *
 * @return A reference to the map of file transfers (key: transfer ID or name).
 */
std::map<std::string, FileTransfer>& Server::getFileTransfers()
{
    return _fileTransfers;
}


/**
 * @brief Retrieves the server name.
 *
 * @return A constant reference to the server's name.
 */
const std::string& Server::getServerName() const
{
    return _serverName;
}


/**
 * @brief Prevents clients from re-registering if already authenticated.
 *
 * If the client is in `NOT_REGISTERED` state and a password is required,
 * it sends an error message preventing re-registration.
 *
 * @param fd The file descriptor of the client.
 */

void Server::mayNotRegistered(int fd)
{
    if (!_password.empty() && getClients()[fd]->authState == NOT_REGISTERED)
    {
        std::string reply = "462 :You may not reregister\r\n";
        safeSend(fd, reply);
    }
}


/**
 * @brief Requires a client to provide the correct password before proceeding.
 *
 * Sends an error message and removes the client from the server if they fail to provide
 * a valid password.
 *
 * @param fd The file descriptor of the client.
 */
void Server::passRequired(int fd)
{
    safeSend(fd, "464 :Password required\r\n");
    removeClient(fd);
}


/**
 * @brief Notifies a client that they have not yet registered.
 *
 * Sends an error message to the client if they attempt an action requiring registration.
 *
 * @param fd The file descriptor of the client.
 */
void Server::notRegistered(int fd)
{
    safeSend(fd, "451 :You have not registered\r\n");
}
