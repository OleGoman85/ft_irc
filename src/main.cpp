/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 12:40:05 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/15 07:38:56 by ogoman           ###   ########.fr       */
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
    try {
        port = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Invalid port number.\n";
        return EXIT_FAILURE;
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
                Сервер запущен на порту 6667

    term2  nc 127.0.0.1 6667
                Hello Server

    nc = (Netcat)
*/