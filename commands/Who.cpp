#include "Who.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include <sstream>
#include <string>
#include <sys/socket.h>

void handleWhoCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) {
    (void)command;
    std::string reply;
    std::string target;
    
    if (tokens.size() >= 2) {
        target = tokens[1];
    }
    
    if (!target.empty() && target[0] == '#') {
        auto it = server->getChannels().find(target);
        if (it == server->getChannels().end()) {
            reply = "403 " + target + " :No such channel\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        for (int client_fd : it->second.getClients()) {
            Client* client = server->getClients()[client_fd].get();
            std::ostringstream oss;

            oss << "352 " << server->getClients()[fd]->getNickname() << " " << target << " " 
                << client->getUsername() << " " << client->getHost() << " " 
                << server->getServerName() << " " << client->getNickname() 
                << " H :0 " << client->getUsername() << "\r\n";
            reply = oss.str();
            send(fd, reply.c_str(), reply.size(), 0);
        }
    }
    else {
        for (const auto& pair : server->getClients()) {
            Client* client = pair.second.get();
            std::ostringstream oss;
            oss << "352 " << server->getClients()[fd]->getNickname() << " * " 
                << client->getUsername() << " " << client->getHost() << " " 
                << server->getServerName() << " " << client->getNickname() 
                << " H :0 " << client->getUsername() << "\r\n";
            reply = oss.str();
            send(fd, reply.c_str(), reply.size(), 0);
        }
    }
    
    reply = "315 " + server->getClients()[fd]->getNickname() + " :End of WHO list\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}
