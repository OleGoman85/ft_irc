#ifndef FILECOMMAND_HPP
#define FILECOMMAND_HPP

#include <string>
#include <vector>
#include "../include/Server.hpp"

/**
 * @brief Handles the IRC "FILE" command (a custom extension for file transfer).
 *
 * Subcommands format:
 *   FILE SEND <nickname> <filename> <filesize>
 *   FILE DATA <filename> <base64_chunk>
 *   FILE END  <filename>
 */
void handleFileCommand(Server* server,
                       int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& fullCommand);

#endif // FILECOMMAND_HPP
