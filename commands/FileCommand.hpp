#ifndef FILECOMMAND_HPP
#define FILECOMMAND_HPP

#include <string>
#include <vector>
#include "../include/Server.hpp"

/**
 * @brief Обработчик IRC-команды FILE (расширение для file transfer).
 *
 * Формат подкоманд (условно):
 *  FILE SEND <nickname> <filename> <filesize>
 *  FILE DATA <filename> <base64_chunk>
 *  FILE END  <filename>
 */
void handleFileCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& fullCommand);

#endif // FILECOMMAND_HPP
