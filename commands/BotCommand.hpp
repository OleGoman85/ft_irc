#ifndef BOTCOMMAND_HPP
#define BOTCOMMAND_HPP

#include <string>
#include <vector>

class Server;

void handleBotCommand(Server* server, int fd,
                      const std::vector<std::string>& tokens,
                      const std::string&              fullCommand);

#endif
