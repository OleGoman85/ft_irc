/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:43:31 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/27 09:32:52 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


//? setupServer()

//!socket
/*
int socket(int domain, int type, int protocol);

Описание: Функция socket используется для создания нового сокета, который является абстракцией для сетевых коммуникаций между двумя узлами в сети.

Параметры:

domain: Определяет семейство протоколов (например, AF_INET для IPv4).
type: Тип сокета (например, SOCK_STREAM для потоковых соединений).
protocol: Протокол, используемый в сокете (обычно устанавливается в 0 для выбора протокола по умолчанию).
Возвращаемое значение: Возвращает файловый дескриптор нового сокета или -1 в случае ошибки.

socket(AF_INET, SOCK_STREAM, 0):

Создает сокет.
AF_INET: Используется семейство адресов IPv4.
SOCK_STREAM: Указывает на использование потокового сокета (например, TCP).
0: Выбор протокола по умолчанию (TCP для SOCK_STREAM).
Возвращает файловый дескриптор сокета или -1 при ошибке.
if (_listen_fd < 0):

Проверяется, успешно ли был создан сокет.
Если значение меньше 0, выбрасывается исключение с сообщением об ошибке.

*/

//!AF_INET
/*
socket(AF_INET, SOCK_STREAM, 0);

Описание: AF_INET (Address Family Internet) — это семейство адресов, используемое для адресации в протоколе IPv4.

Использование: Применяется при создании сокетов для IPv4 адресации.
*/


//!SOCK_STREAM TCP
/*
socket(AF_INET, SOCK_STREAM, 0);

Описание: SOCK_STREAM — тип сокета, обеспечивающий надежное, ориентированное на соединение потоковое взаимодействие. Используется, например, в протоколе TCP.

Особенности:

Гарантирует доставку данных без потерь и в правильном порядке.
Поддерживает двунаправленную связь.
*/


//!setsockopt
/*
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

Описание: Функция setsockopt используется для установки опций на сокетах. Позволяет изменять поведение сокета, например, устанавливать таймауты или управлять параметрами буферов.

Параметры:
sockfd: Дескриптор сокета.
level: Уровень, на котором устанавливается опция (например, SOL_SOCKET).
optname: Имя опции (например, SO_REUSEADDR).
optval: Указатель на значение опции.
optlen: Размер значения опции.
Возвращаемое значение: Возвращает 0 при успехе и -1 при ошибке.
*/


//! opt = 1:
/*Устанавливаем значение 1 для разрешения повторного использования адреса и порта.
setsockopt:

Настраивает параметры сокета.
SOL_SOCKET: Указывает уровень настроек для самого сокета.
SO_REUSEADDR: Опция, позволяющая повторно использовать адрес и порт, даже если они находятся в состоянии TIME_WAIT.
&opt: Указатель на значение опции.
sizeof(opt): Размер значения опции.
Проверка результата:

Если setsockopt возвращает < 0, возникает ошибка, и выбрасывается исключение.
*/


//!SOL_SOCKET
/*
SOL_SOCKET
Описание: SOL_SOCKET — уровень протокола для опций, связанных с самим сокетом. Используется в функциях setsockopt и getsockopt для задания или получения общих опций сокета.

Примеры опций на этом уровне:

SO_REUSEADDR
SO_KEEPALIVE
SO_RCVBUF
*/


//!SO_REUSEADDR
/*
int opt = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


SO_REUSEADDR
Описание: SO_REUSEADDR — опция сокета, позволяющая повторно использовать локальный адрес (IP и порт), даже если он недавно использовался другим сокетом.

Применение: Полезно при перезапуске серверного приложения, чтобы избежать ошибки "Address already in use".
*/


//! fcntl
/*
int fcntl(int fd, int cmd, ...);

Описание: Функция fcntl используется для управления файловыми дескрипторами. Позволяет изменять различные атрибуты дескриптора, такие как режим блокировки или флаги.

Параметры:

fd: Файловый дескриптор.
cmd: Команда для выполнения (например, F_SETFL).
Дополнительные аргументы зависят от команды.
Возвращаемое значение: Зависит от команды. При ошибке возвращает -1.

fcntl:
Управляет файловыми дескрипторами.
F_SETFL: Устанавливает флаги файлового дескриптора.
O_NONBLOCK: Включает неблокирующий режим для сокета.
В неблокирующем режиме функции чтения/записи не будут блокировать выполнение, если данные недоступны.
Проверка результата:

Если настройка режима завершилась с ошибкой, выбрасывается исключение.
*/


//! F_SETFL
/*
fcntl(sockfd, F_SETFL, O_NONBLOCK);

Описание: F_SETFL — команда для функции fcntl, используемая для установки флагов состояния файлового дескриптора.

Применение: Часто используется для установки неблокирующего режима (O_NONBLOCK) на сокетах или файловых дескрипторах.
*/


//! O_NONBLOCK
/*
fcntl(sockfd, F_SETFL, O_NONBLOCK);

Описание: O_NONBLOCK — флаг, устанавливающий неблокирующий режим для файлового дескриптора. В неблокирующем режиме операции ввода-вывода не будут блокировать выполнение программы, если данные недоступны.

Применение: Используется для сокетов, чтобы позволить программе обрабатывать другие задачи, пока данные не станут доступными.
*/


//! sockaddr_in
/*
Описание: Структура sockaddr_in используется для хранения адресной информации для IPv4.

Поля:

sin_family: Семейство адресов (AF_INET).
sin_port: Номер порта, преобразованный с помощью htons.
sin_addr: Структура in_addr, содержащая IP-адрес (например, INADDR_ANY).

struct sockaddr_in {
        short   sin_family;         //address family
        u_short sin_port;           //16 bit TCP/UDP port number
        struct  in_addr sin_addr;   //32 bit IP address
        char    sin_zero[8];        //not use, for align
};
*/


//!INADDR_ANY
/*
addr.sin_addr.s_addr = INADDR_ANY;

Указывает, что сокет будет принимать подключения на всех доступных IP-адресах.

Описание: INADDR_ANY — специальное значение для поля sin_addr.s_addr структуры sockaddr_in, означающее, что сокет будет привязан к всем доступным интерфейсам на машине.

Применение: Позволяет серверу принимать подключения на любом IP-адресе, присвоенном хосту.
*/


//!htons
/*
uint16_t htons(uint16_t hostshort);

Применение: Используется для преобразования номера порта перед установкой в структуру sockaddr_in.
addr.sin_port = htons(port);

Описание: Функция htons (Host TO Network Short) преобразует 16-битное целое число из порядка байтов хоста в сетевой порядок байтов (Big Endian).
*/


//!bind
/*
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

Описание: Функция bind привязывает сокет к определенному адресу и порту, указанным в структуре sockaddr.

Параметры:
sockfd: Дескриптор сокета.
addr: Указатель на структуру sockaddr с адресом и портом.
addrlen: Размер структуры sockaddr.
Возвращаемое значение: Возвращает 0 при успехе и -1 при ошибке.
*/


//!sockaddr
/*
struct sockaddr {
    sa_family_t sa_family; // Семейство адресов
    char        sa_data[14]; // Данные адреса
};

Описание: struct sockaddr — обобщенная структура для хранения адресной информации. Используется в функциях сокетов как универсальный тип для различных семейств адресов.

Применение: Преобразуется из более специализированных структур, таких как sockaddr_in для IPv4.
*/


//!listen
/*
int listen(int sockfd, int backlog);

Описание: Функция listen переводит сокет в состояние прослушивания, позволяя ему принимать входящие соединения.

Параметры:

sockfd: Дескриптор сокета, привязанного к адресу и порту.
backlog: Максимальное количество ожидающих соединений в очереди.
Возвращаемое значение: Возвращает 0 при успехе и -1 при ошибке.
*/


//!SOMAXCONN
/*
Описание: SOMAXCONN — константа, определяющая максимальное количество ожидающих соединений в очереди для функции listen. Значение определяется системой.

Применение: Используется как параметр backlog в функции listen для установки максимальной длины очереди ожидающих соединений.
*/


//!pollfd
/*
struct pollfd {
    int   fd;         // Файловый дескриптор
    short events;     // События, которые необходимо отслеживать
    short revents;    // События, которые произошли
};

Описание: struct pollfd используется с функцией poll для мониторинга нескольких файловых дескрипторов на наличие событий ввода-вывода.

Поля:

fd: Файловый дескриптор для мониторинга.
events: Битовая маска событий, которые нужно отслеживать (например, POLLIN).
revents: Битовая маска событий, которые произошли.
*/


//!POLLIN
/*
Описание: POLLIN — флаг события для функции poll, указывающий, что на файловом дескрипторе доступны данные для чтения.

Применение: Используется в поле events структуры pollfd, чтобы указать, что необходимо отслеживать наличие входящих данных.*/



//? Server::run()

/**
 * @brief Запускает основной цикл сервера.
 * 
 * Ожидает и обрабатывает события с помощью функции poll. Принимает новые соединения
 * и обрабатывает данные от клиентов.
 * 
//! -1:
    Таймаут в миллисекундах. Значение -1 означает, что функция будет ждать событий бесконечно.

//! poll (модуль ядра):
    Отслеживает события на сокетах.
    Используется для асинхронной работы сервера.
    Позволяет серверу ожидать событий (например, данных для чтения, готовности к записи или ошибок) на нескольких файловых дескрипторах, не блокируя выполнение программы.

 //! _poll_fds.data() - Указатель на массив структур pollfd
 * poll_fds (вектор pollfd):
    Хранит все отслеживаемые дескрипторы сокетов.
    Каждый элемент описывает сокет, события, которые нужно отслеживать, и события, которые произошли.

//! acceptNewConnection:
    Обрабатывает новые подключения, добавляя их в массив _poll_fds и регистрируя в системе сервера.
 
//! handleClientData:
    Читает данные от клиента, сохраняет их в буфер и обрабатывает команды.

//! data
    это метод или член данных контейнеров C++, таких как std::vector или std::array. Он возвращает указатель на внутренний массив данных контейнера.
    Для получения указателя на первый элемент массива, управляемого контейнером.

//! POLLIN
     указывает на событие "доступны данные для чтения".
     Описание: POLLIN — флаг события для функции poll, указывающий, что на файловом дескрипторе доступны данные для чтения.

    Применение: Используется в поле events структуры pollfd, чтобы указать, что необходимо отслеживать наличие входящих данных.

//! revents: 
    Поле, которое указывает на события, произошедшие на файловом дескрипторе.
*/


//? acceptNewConnection()

/**
 * @brief Обрабатывает новое входящее соединение.
 * 
 * Принимает новое подключение, переводит его в неблокирующий режим и
 * добавляет в список отслеживаемых дескрипторов.
 * 
 //! struct sockaddr_in client_addr
 * Создается структура sockaddr_in для хранения информации о клиенте, который пытается подключиться. будет записан IP-адрес и порт клиента после успешного вызова accept.
 * 
 //! socklen_t client_len = sizeof(client_addr):
 * Задает длину структуры client_addr. Это требуется для передачи в функцию accept, чтобы она знала, сколько места выделено для записи данных клиента.
 * 
 //! accept()
 * Системный вызов, который принимает новое входящее подключение на серверный сокет _listen_fd.
 * 
 * Если успешно: возвращает файловый дескриптор (client_fd) для общения с клиентом.
 
 *  
//!sockaddr
struct sockaddr {
    sa_family_t sa_family; // Семейство адресов
    char        sa_data[14]; // Данные адреса
};

Описание: struct sockaddr — обобщенная структура для хранения адресной информации. Используется в функциях сокетов как универсальный тип для различных семейств адресов.

Применение: Преобразуется из более специализированных структур, таких как sockaddr_in для IPv4.

 * 
 //! F_SETFL: 
    Устанавливает новые флаги для дескриптора.
 * 
 //!O_NONBLOCK: 
    Переводит сокет в неблокирующий режим, чтобы операции ввода-вывода не блокировали выполнение программы.
 * 
 //! _clients
 * контейнер std::map, где хранятся данные обо всех подключенных клиентах.
 * 
 //!inet_ntoa(client_addr.sin_addr):
 * Преобразует IP-адрес клиента из структуры sockaddr_in в строковый вид (например, 192.168.0.1).
 * 
 //! ntohs(client_addr.sin_port):
 * Преобразует порт клиента из сетевого порядка байтов в порядок хоста.
 * 
 //! emplace
 *  Добавляет новый элемент в _clients
 *  Ключ: client_fd (файловый дескриптор клиента).
 *  Значение: Указатель std::unique_ptr<Client>.
 * метод контейнеров STL. который создает и добавляет новый элемент в контейнер на месте, без дополнительных копирований или перемещений объектов. Вместо того чтобы создавать объект заранее, а затем передавать его в контейнер, emplace создает объект непосредственно внутри контейнера.
 
 * 
 //! std::make_unique
 * используется для создания объекта в динамической памяти и оборачивания его в умный указатель std::unique_ptr.
 * std::unique_ptr<Client> client(new Client(client_fd));
 * auto client = std::make_unique<Client>(client_fd);
 * 
 */




//? handleClientData
/**
 * @brief Обрабатывает данные от клиента.
 * 
 * Читает данные из сокета клиента, добавляет их в его буфер и 
 * обрабатывает команды, завершенные символом новой строки.
 * 
 * @param fd Файловый дескриптор клиента.
 * 
//! recv
    Системный вызов, используемый для чтения данных из сокета.

//! removeClient(fd);
    Функция removeClient закрывает сокет клиента и удаляет его из списка _clients и _poll_fds.

//! substr
    это метод класса std::string, который возвращает подстроку из строки.
    
//! processCommand
    это метод, который обрабатывает извлеченную из буфера строку (команду).
*/


//! c_str
/*
    это метод класса std::string, который возвращает указатель на C-строку (массив символов с завершающим нулем \0).

    Зачем нужен c_str?
    C++ строки (std::string) — это высокоуровневый тип данных, который удобно использовать в современном коде.
    Однако функции уровня C (например, send, printf и т. д.) работают с низкоуровневыми строками — массивами символов (char*), которые заканчиваются символом \0.
    Метод c_str преобразует std::string в const char*, чтобы он был совместим с C-функциями.

*/




#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../commands/Nick.hpp"
#include "../commands/Pass.hpp"
#include "../commands/User.hpp"
#include "../commands/Join.hpp"
#include "../commands/Privmsg.hpp"
#include "../commands/Quit.hpp"
#include "../commands/Kick.hpp"
#include "../commands/Invite.hpp"
#include "../commands/Topic.hpp"
#include "../commands/Mode.hpp"




/**
 * @brief Server constructor.
 *
 * Initializes the server with the given port and password.
 * Sets up internal data structures and calls setupServer() to configure the listening socket.
 *
 * @param port The port number on which the server listens.
 * @param password The connection password required for clients.
 */
Server::Server(int port, const std::string& password)
    : _password(password), //Sets the value of _password.
      _clients(), //Initializes the _clients map to hold connected clients.
      _channels(), //Initializes the _channels map to hold channels.
      _port(port), //Sets the port the server will use.
      _listen_fd(-1) // Sets the file descriptor for the listening socket.
{setupServer();} //The setupServer method sets up the server socket for operation.

/**
 * @brief Server destructor.
 *
 * Closes the listening socket if it is open.
 */
Server::~Server() 
{
    if (_listen_fd != -1)
        close(_listen_fd);
}

/**
 * @brief Sets up the server socket.
 *
 * Creates a non-blocking socket, sets socket options, binds it to the specified port,
 * and starts listening for incoming connections. The listening socket is added to the poll descriptor vector.
 *
 * @throws std::runtime_error if any socket operation fails.
 */
void Server::setupServer() 
{
    //configures the server socket.
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP, 0
    if (_listen_fd < 0)
        throw std::runtime_error("Failed to create socket");

    //Allows the address and port to be reused even if the socket is not closed correctly.
    int opt = 1; 
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt failed");
    
    // Setting non-blocking mode
    if (fcntl(_listen_fd, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to set non-blocking mode");

    //Structure for storing address information (IPv4)
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind failed");
    
    // Start listening
    if (listen(_listen_fd, SOMAXCONN) < 0)
        throw std::runtime_error("listen failed");

    // Structure for tracking socket events.
    struct pollfd pfd;
    pfd.fd = _listen_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _poll_fds.push_back(pfd);

    std::cout << "Server started on port " << _port << "\n";
}

/**
 * @brief Runs the main server loop.
 *
 * Continuously polls for events on the listening socket and client sockets.
 * When data is available, calls acceptNewConnection() for new connections or handleClientData() for existing clients.
 */

void Server::run() 
{
    while (true)  // Infinite loop, running as long as the server is active.
    {
        // Call poll to monitor events on all sockets.
        int poll_count = poll(_poll_fds.data(), _poll_fds.size(), -1);

        // Check if poll returned an error (negative value).
        if (poll_count < 0) {
            std::cerr << "poll error\n";  // Log the error message.
            continue;  // Skip to the next iteration of the loop.
        }

        // Iterate over all file descriptors in the _poll_fds array.
        for (size_t i = 0; i < _poll_fds.size(); ++i) 
        {
            // Check if the event we're interested in (POLLIN) occurred.
            if (_poll_fds[i].revents & POLLIN) 
            {
                // If the event occurred on the listening socket (_listen_fd),
                // it means there is a new incoming connection.
                if (_poll_fds[i].fd == _listen_fd)
                    acceptNewConnection();  // Handle the new connection.
                else
                    // Otherwise, handle incoming data from the client socket.
                    handleClientData(_poll_fds[i].fd);
            }
        }
    }
}


/**
 * @brief Accepts a new client connection.
 *
 * Accepts a new connection on the listening socket, 
 * sets it to non-blocking mode,
 * adds it to the poll descriptor vector, 
 * and creates a new Client object to manage the connection.
 */
void Server::acceptNewConnection() 
{
    // Create a structure to hold the client's address and its length
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Accept a new connection from a client
    int client_fd = accept(_listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        // Check if the error is not due to non-blocking mode and print an error if it's a real issue
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            std::cerr << "accept failed\n";
        return; // Exit the function if no connection was accepted
    }

    // Set the new client's socket to non-blocking mode
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Failed to set non-blocking mode for client\n";
        close(client_fd); // Close the client's socket if the operation fails
        return;
    }

    // Add the client's socket descriptor to the poll structure
    struct pollfd pfd;
    pfd.fd = client_fd;       // File descriptor for the client's socket
    pfd.events = POLLIN;      // Interested in "data ready to read" events
    pfd.revents = 0;          // Reset the returned events field
    _poll_fds.push_back(pfd); // Add the poll descriptor to the poll vector

    // Add the client to the clients map using their file descriptor
    _clients.emplace(client_fd, std::make_unique<Client>(client_fd));

    // Log the new connection with the client's IP and port
    std::cout << "New connection from " 
              << inet_ntoa(client_addr.sin_addr)   // Convert client's IP to a readable format
              << ":" << ntohs(client_addr.sin_port) // Convert client's port to host byte order
              << " (fd: " << client_fd << ")\n";
}



/**
 * @brief Handles incoming data from a client.
 *
 * Reads data from the client socket in a non-blocking manner.
 * Aggregates the data in the client's buffer, and when a complete command (terminated by "\r\n" or "\n") is received,
 * it extracts and processes the command.
 *
 * @param fd File descriptor of the client.
 */
void Server::handleClientData(int fd) {
    char buffer[512];
    int bytes_received = recv(fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0)
            std::cout << "Client (fd: " << fd << ") disconnected\n";
        else
            std::cerr << "recv error on fd " << fd << "\n";
        removeClient(fd);
        return;
    }
    // Append received data to the client's buffer.
    _clients[fd]->buffer.append(buffer, bytes_received);

    size_t pos;
    while (true) {
        // Search for the command delimiter "\r\n" (or "\n" as fallback).
        pos = _clients[fd]->buffer.find("\r\n");
        if (pos == std::string::npos) {
            pos = _clients[fd]->buffer.find("\n");
            if (pos != std::string::npos && pos > 0 && _clients[fd]->buffer[pos - 1] == '\r') {
                pos -= 1;
            }
        }
        // If no delimiter is found, break out of the loop.
        if (pos == std::string::npos)
            break;

        // Extract the command up to the delimiter.
        std::string command = _clients[fd]->buffer.substr(0, pos);
        
        // Remove the command and its delimiter from the buffer.
        if (_clients[fd]->buffer.substr(pos, 2) == "\r\n")
            _clients[fd]->buffer.erase(0, pos + 2);
        else
            _clients[fd]->buffer.erase(0, pos + 1);

        // Trim whitespace from the beginning and end of the command.
        command.erase(0, command.find_first_not_of(" \t"));
        command.erase(command.find_last_not_of(" \t") + 1);

        processCommand(fd, command);
        if (_clients.find(fd) == _clients.end())
            break;
    }
}

/**
 * @brief Removes a client from the server.
 *
 * Closes the client socket, removes the client from the server's client map,
 * and removes the corresponding poll descriptor from the poll vector.
 *
 * @param fd File descriptor of the client to be removed.
 */
void Server::removeClient(int fd) {
    close(fd);
    _clients.erase(fd);
    for (size_t i = 0; i < _poll_fds.size(); ++i) {
        if (_poll_fds[i].fd == fd) {
            _poll_fds.erase(_poll_fds.begin() + i);
            break;
        }
    }
}

/**
 * @brief Broadcasts a message to all clients except the sender.
 *
 * Iterates over the client map and sends the provided message to each client whose file descriptor is not equal to sender_fd.
 *
 * @param message The message to be broadcast.
 * @param sender_fd File descriptor of the sender (this client will be excluded from receiving the message).
 */
void Server::broadcastMessage(const std::string& message, int sender_fd) 
{
    for (const auto& pair : _clients) {
        int client_fd = pair.first;
        if (client_fd != sender_fd)
            send(client_fd, message.c_str(), message.size(), 0);
    }
}

/**
 * @brief Processes a complete command received from a client.
 *
 * Splits the command string into tokens and dispatches it to the appropriate command handler based on the first token.
 *
 * @param fd File descriptor of the client that sent the command.
 * @param command The complete command string.
 */
void Server::processCommand(int fd, const std::string& command) {
    std::cout << "Command from fd " << fd << ": " << command << std::endl;
    std::vector<std::string> tokens = Utils::split(command, ' ');
    if (tokens.empty())
        return;
    std::string cmd = tokens[0];

    if (cmd == "PASS") {
        handlePassCommand(this, fd, tokens, command);
    }
    else if (cmd == "NICK") {
        handleNickCommand(this, fd, tokens, command);
    }
    else if (cmd == "USER") {
        handleUserCommand(this, fd, tokens, command);
    }
    else if (cmd == "JOIN") {
        handleJoinCommand(this, fd, tokens, command);
    }
    else if (cmd == "PRIVMSG") {
        handlePrivmsgCommand(this, fd, tokens, command);
    }
    else if (cmd == "QUIT") {
        handleQuitCommand(this, fd, tokens, command);
    }
    else if (cmd == "KICK") {
        handleKickCommand(this, fd, tokens, command);
    }
    else if (cmd == "INVITE") {
        handleInviteCommand(this, fd, tokens, command);
    }
    else if (cmd == "TOPIC") {
        handleTopicCommand(this, fd, tokens, command);
    }
    else if (cmd == "MODE") {
        handleModeCommand(this, fd, tokens, command);
    }
    else {
        std::string reply = "421 " + cmd + " :Unknown command\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
    }
}



//! POOL
/*
poll — это системный вызов (из библиотеки <poll.h> в C/C++), который используется для мониторинга нескольких файловых дескрипторов (например, сокетов) на наличие событий. Он позволяет эффективно обрабатывать множественные соединения без необходимости создания множества потоков.

Как работает poll?
Вы создаете массив структур struct pollfd, где каждая структура представляет один файловый дескриптор (например, сокет) и интересующие вас события.
Вызываете функцию poll, которая блокируется (или возвращает сразу, если указано время ожидания), пока на каком-либо из файловых дескрипторов не произойдут события.
После возвращения poll проверяете, какие события произошли на дескрипторах, и обрабатываете их.
*/
