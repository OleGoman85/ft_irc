#ifndef PART_HPP
#define PART_HPP

#include <string>
#include <vector>

class Server;
void handlePartCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // PART_HPP
