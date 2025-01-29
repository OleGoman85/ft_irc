/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:40:05 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/29 07:38:23 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include <iostream>
#include <cstdlib>


int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return EXIT_FAILURE;
    }

    int port;
    if(std::string(argv[1]) == "port")
    {
        port = 6667;
    }
    else
    {
        try {
            port = std::stoi(argv[1]);
            // Check if the port is within the valid range
            if (port < 1024 || port > 65535) {
                std::cerr << "Error: Port must be in the range 1024-65535.\n";
                return EXIT_FAILURE;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number.\n";
            return EXIT_FAILURE;
        }
    }

    std::string password = argv[2];
    
    try {
        Server server(port, password);
        server.run();                    //! start server
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


/*
    term1  ./ircserv 6667 mypassword
            ./ircserv port mypassword
                Сервер запущен на порту 6667
                

    term2  nc 127.0.0.1 6667
            nc localhost 6667
                Hello Server

    nc = (Netcat)
*/

/*
Стандарт IRC
Порт 6667 был определён как стандартный порт для IRC-серверов. Большинство IRC-клиентов по умолчанию пытаются подключиться к серверам именно через этот порт.

Однако можно использовать любой порт, который:
Не конфликтует с другими службами.
Находится в диапазоне 1024-65535 (порты ниже 1024 требуют привилегий суперпользователя).

*/


/*
127.0.0.1 — это localhost
Это зарезервированный IP-адрес, который используется для подключения к серверу на том же устройстве, где он запущен. Он идеален для тестирования, поскольку:

Трафик не выходит за пределы устройства.
Быстрая настройка.
*/