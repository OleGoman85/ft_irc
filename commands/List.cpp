#include "List.hpp"
#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include <sstream>
#include <string>
#include <sys/socket.h>

void handleListCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command) {
    (void)tokens;
    (void)command;
    std::string reply;
    for (auto& pair : server->getChannels()) {
        Channel& channel = pair.second;
        std::ostringstream oss;
        oss << "322 " << server->getClients()[fd]->getNickname() << " " << channel.getName() 
            << " " << channel.getClients().size() << " :" << channel.getTopic() << "\r\n";
        reply = oss.str();
        send(fd, reply.c_str(), reply.size(), 0);
    }
    reply = "323 " + server->getClients()[fd]->getNickname() + " :End of LIST\r\n";
    send(fd, reply.c_str(), reply.size(), 0);
}
