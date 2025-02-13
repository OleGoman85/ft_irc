#include "Whois.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include <sstream>
#include <string>
#include <sys/socket.h>

void handleWhoisCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) {
    (void)command;
    if (tokens.size() < 2) {
        std::string reply = "461 WHOIS :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    std::string targetNick = tokens[1];
    Client* targetClient = nullptr;
    for (auto& pair : server->getClients()) {
        if (pair.second->getNickname() == targetNick) {
            targetClient = pair.second.get();
            break;
        }
    }
    if (!targetClient) {
        std::string reply = "401 " + targetNick + " :No such nick/channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    
    std::ostringstream oss;
    std::string realName = targetClient->getRealName();
    if (realName.empty()) {
         realName = "Real name not set";
    }
    oss << "311 " << server->getClients()[fd]->getNickname() << " " 
        << targetNick << " " << targetClient->getUsername() << " " 
        << targetClient->getHost() << " * :" << realName << "\r\n";
    std::string reply = oss.str();
    send(fd, reply.c_str(), reply.size(), 0);
    
    reply = "318 " + server->getClients()[fd]->getNickname() + " " + targetNick + " :End of WHOIS\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}
