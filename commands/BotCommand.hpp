#ifndef BOTCOMMAND_HPP
#define BOTCOMMAND_HPP

#include <string>
#include <vector>

class Server;

/**
 * @brief Обработчик команды BOT.
 *
 * Доступные подкоманды:
 * - JOIN #канал
 * - LEAVE #канал
 * - SAY #канал сообщение
 * - 8BALL вопрос
 */
void handleBotCommand(Server* server, int fd,
                      const std::vector<std::string>& tokens,
                      const std::string&              fullCommand);

#endif  // BOTCOMMAND_HPP
