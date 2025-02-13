#include "Replies.hpp"

#include <sys/socket.h>

#include <string>

#include "../include/Client.hpp"
#include "../include/Server.hpp"

void sendWelcome(Server* server, int fd)
{
    Client*     client = server->getClients()[fd].get();
    std::string nick   = client->getNickname();
    std::string srv    = server->getServerName();

    // 001: RPL_WELCOME
    std::string rpl1 = ":" + srv + " 001 " + nick + " :Welcome to " + srv +
                       ", " + nick + "!\r\n";
    // 002: RPL_YOURHOST
    std::string rpl2 = ":" + srv + " 002 " + nick + " :Your host is " + srv +
                       ", running version 1.0\r\n";
    // 003: RPL_CREATED
    std::string rpl3 =
        ":" + srv + " 003 " + nick + " :This server was created just now\r\n";
    // 004: RPL_MYINFO
    std::string rpl4 =
        ":" + srv + " 004 " + nick + " " + srv + " 1.0 iwtov\r\n";

    send(fd, rpl1.c_str(), rpl1.size(), 0);
    send(fd, rpl2.c_str(), rpl2.size(), 0);
    send(fd, rpl3.c_str(), rpl3.size(), 0);
    send(fd, rpl4.c_str(), rpl4.size(), 0);
}
