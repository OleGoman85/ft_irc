/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alisa <alisa@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:42:09 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/08 06:36:51 by alisa            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <sys/socket.h>
#include <vector>
#include <map>
#include <memory>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"
#include "FileTransfer.hpp"

/**
 * @brief Represents an IRC server.
 *
 * The Server class encapsulates the functionality of an IRC server.
 * It manages the listening socket, accepts new client connections,
 * handles client data using a poll-based event loop,
 * and dispatches commands to appropriate handlers.
 */
class Server {
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
    ~Server();

    /**
     * @brief Runs the main server loop.
     *
     * Starts the server's event loop, which continuously polls for events
     * (new connections, incoming data) and dispatches them to the appropriate handlers.
     */
    void run();

    /**
     * @brief Removes a client from the server.
     *
     * Closes the client socket, removes the client from the internal client map,
     * and removes its poll descriptor.
     *
     * @param fd The file descriptor of the client to be removed.
     */
    void removeClient(int fd);

    /**
     * @brief Broadcasts a message to all connected clients except the sender.
     *
     * Iterates over the internal client map and sends the message to every client whose file descriptor
     * does not match the sender's.
     *
     * @param message The message to broadcast.
     * @param sender_fd The file descriptor of the sender (this client will not receive the message).
     */
    void broadcastMessage(const std::string& message, int sender_fd);

        /**
     * @brief Returns current server password (read-only).
     */
    const std::string& getPassword() const;

    /**
     * @brief Changes the server password.
     */
    void setPassword(const std::string& newPassword);

    /**
     * @brief Returns the map of all connected clients.
     */
    std::map<int, std::unique_ptr<Client>>& getClients();

    /**
     * @brief Returns the map of all channels.
     */
    std::map<std::string, Channel>& getChannels();

    std::map<std::string, FileTransfer>& getFileTransfers();


private:
    int _port;                     ///< The port number on which the server listens.
    int _listen_fd;                ///< The listening socket file descriptor.
    std::vector<struct pollfd> _poll_fds; ///< Vector of poll descriptors for the server and client sockets.
    
    std::string _password;                         ///< The connection password for the server.
    std::map<int, std::unique_ptr<Client>> _clients; ///< Map of client file descriptors to Client objects.
    std::map<std::string, Channel> _channels;        ///< Map of channel names to Channel objects.

    std::map<std::string, FileTransfer> _fileTransfers; 
    // где ключ — это некий "senderFd_filename",
    // а значение — сам объект FileTransfer

    /**
     * @brief Sets up the server's listening socket.
     *
     * Creates a non-blocking socket, sets necessary options,
     * binds it to the specified port, and begins listening for connections.
     *
     * @throws std::runtime_error if any socket operations fail.
     */
    void setupServer();

    /**
     * @brief Accepts a new client connection.
     *
     * Accepts a new connection from the listening socket, sets it to non-blocking mode,
     * adds it to the poll vector, and creates a new Client object to manage the connection.
     */
    void acceptNewConnection();

    /**
     * @brief Handles incoming data from a client.
     *
     * Reads available data from the client's socket, aggregates it in the client's buffer,
     * and processes complete commands separated by "\r\n" (or "\n").
     *
     * @param fd The file descriptor of the client whose data is to be handled.
     */
    void handleClientData(int fd);

    /**
     * @brief Processes a complete command received from a client.
     *
     * Splits the command into tokens and dispatches it to the appropriate command handler.
     *
     * @param fd The file descriptor of the client that sent the command.
     * @param command The complete command string received from the client.
     */
    void processCommand(int fd, const std::string& command);
};

#endif // SERVER_HPP







/*
//!Поля:
_port – порт, на котором слушает сервер.
_password – пароль для подключения клиентов.
_listen_fd – файловый дескриптор серверного сокета.
_poll_fds – вектор структур pollfd для мониторинга событий на сокетах.
_clients – карта подключённых клиентов, где ключ – дескриптор сокета, значение – уникальный указатель на объект Client.
_channels – карта каналов, где ключ – имя канала, значение – объект Channel.

//!Методы:
setupServer() – настройка серверного сокета (создание, настройка опций, привязка, прослушивание).
run() – основной цикл сервера, использующий poll для мониторинга событий.
acceptNewConnection() – принятие нового подключения от клиента.
handleClientData(int fd) – обработка данных, полученных от клиента.
removeClient(int fd) – удаление клиента из сервера.
broadcastMessage(const std::string& message, int sender_fd) – отправка сообщения всем клиентам, кроме отправителя.
processCommand(int fd, const std::string& command) – обработка команды, полученной от клиента.
*/


/*
//! Fields:
_port – the port on which the server is listening.
_password – the password required for clients to connect.
_listen_fd – the file descriptor of the server socket.
_poll_fds – a vector of pollfd structures for monitoring events on sockets.
_clients – a map of connected clients, where the key is the socket descriptor, and the value is a unique pointer to a Client object.
_channels – a map of channels, where the key is the channel name, and the value is a Channel object.

//! Methods:
setupServer() – configures the server socket (creation, setting options, binding, and listening).
run() – the main server loop using poll to monitor events.
acceptNewConnection() – accepts a new connection from a client.
handleClientData(int fd) – processes data received from a client.
removeClient(int fd) – removes a client from the server.
broadcastMessage(const std::string& message, int sender_fd) – sends a message to all clients except the sender.
processCommand(int fd, const std::string& command) – processes a command received from a client.

*/