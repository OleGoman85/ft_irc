#include <sstream>
#include <stdexcept>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

/**
 * @brief Обработчик команды MODE для каналов.
 *
 * Поддерживает основные режимы (i, t, k, l, o), перечисленные в задании.
 * Формат:
 *   MODE #channel               -> показать текущие режимы
 *   MODE #channel +i            -> установить invite-only
 *   MODE #channel -i            -> убрать invite-only
 *   MODE #channel +k <key>      -> установить ключ канала
 *   MODE #channel -k            -> убрать ключ
 *   MODE #channel +l <limit>    -> установить лимит пользователей
 *   MODE #channel -l            -> убрать лимит
 *   MODE #channel +o <nickname> -> дать операторку
 *   MODE #channel -o <nickname> -> снять операторку
 *   MODE #channel +itk <...>    -> можно комбинировать режимы (каждый +/–
 * меняет контекст включения/выключения).
 */
void handleModeCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string&              rawCommand)
{
    (void)
        rawCommand;  // здесь можно было бы парсить "trailing" часть, если нужно

    // 1. Проверяем, что клиент зарегистрирован.
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        // 451 ERR_NOTREGISTERED
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 2. Нужно как минимум: MODE #channel
    if (tokens.size() < 2)
    {
        // 461 ERR_NEEDMOREPARAMS
        std::string reply = "461 MODE :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string channelName = tokens[1];

    // 3. Проверяем, есть ли такой канал
    auto it = server->getChannels().find(channelName);
    if (it == server->getChannels().end())
    {
        // 403 ERR_NOSUCHCHANNEL
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    Channel& channel = it->second;

    // 4. Если передали только "MODE #channel", возвращаем текущие режимы
    if (tokens.size() == 2)
    {
        // Собираем строку режимов в формате "+itk" (если они есть)
        std::string modes;
        if (channel.isInviteOnly()) modes += "i";
        if (channel.isTopicRestricted()) modes += "t";
        if (channel.hasMode('k')) modes += "k";
        if (channel.hasMode('l')) modes += "l";
        // 'o' — режим, применяющийся к конкретному пользователю,
        // обычно как общий флаг не выводится (это не "канальный" глобальный
        // режим, а персоны)

        if (!modes.empty())
            modes = "+" + modes;
        else
            modes = "+";

        // 324 RPL_CHANNELMODEIS <nick> <channel> <modes> [<key/limit>...]
        // Обычно IRC-сервер либо выводит все параметры, либо только первый
        // ключ/лимит: Здесь упростим до вывода одного параметра (если есть).
        std::string reply = "324 " + server->getClients()[fd]->getNickname() +
                            " " + channelName + " " + modes;

        if (channel.hasMode('k'))
            reply += " " + channel.getChannelKey();
        else if (channel.hasMode('l'))
            reply += " " + std::to_string(channel.getUserLimit());

        reply += "\r\n";
        send(fd, reply.c_str(), reply.size(), 0);

        // (по стандарту IRC можно отправлять ещё 329 RPL_CREATIONTIME,
        //  но в задании это не требуется)
        return;
    }

    // 5. Если хотим менять режимы, проверяем, что пользователь - оператор.
    if (!channel.isOperator(fd))
    {
        // 482 ERR_CHANOPRIVSNEEDED
        std::string reply =
            "482 " + channelName + " :You're not a channel operator\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Предположим, что tokens[2] — это строка с режимами (+i, -k, +o и т.д.)
    std::string modeString = tokens[2];

    // Параметры могут идти дальше: tokens[3], tokens[4], ...
    size_t paramIndex = 3;

    // Сформируем, что реально применилось (для единого BROADCAST):
    // Пример итоговой строки: "+ik-l someKey Nick"
    std::ostringstream       appliedModeFlags;
    std::vector<std::string> appliedParams;

    // По спецификации IRC: внутри modeString может быть несколько
    // переключений +/–: "+ikt-l", "+l 10", потом "-k", и т.п.
    bool plus = false;  // по умолчанию не выставляем, ждём первого '+' или '-'

    for (size_t i = 0; i < modeString.size(); ++i)
    {
        char c = modeString[i];

        if (c == '+')
        {
            plus = true;
            continue;
        }
        else if (c == '-')
        {
            plus = false;
            continue;
        }

        // теперь c - это конкретный флаг (i, t, k, l, o), который надо
        // включить(plus=1) или выключить(plus=0)
        switch (c)
        {
            case 'i':  // invite-only
            case 't':  // topic restricted
            {
                channel.setMode(c, plus);
                appliedModeFlags << (plus ? "+" : "-") << c;
            }
            break;

            case 'k':  // канал с ключом
            {
                if (plus)
                {
                    // требуется параметр ключа
                    if (paramIndex >= tokens.size())
                    {
                        // 461 ERR_NEEDMOREPARAMS
                        std::string reply =
                            "461 MODE :Not enough parameters for +k\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                        return;
                    }
                    std::string key = tokens[paramIndex++];
                    channel.setMode('k', true, key);
                    appliedModeFlags << "+k";
                    appliedParams.push_back(key);
                }
                else
                {
                    // убрать ключ, параметр не нужен
                    channel.setMode('k', false);
                    appliedModeFlags << "-k";
                }
            }
            break;

            case 'l':  // limit
            {
                if (plus)
                {
                    // требуется параметр лимита
                    if (paramIndex >= tokens.size())
                    {
                        // 461 ERR_NEEDMOREPARAMS
                        std::string reply =
                            "461 MODE :Not enough parameters for +l\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                        return;
                    }
                    std::string limitStr = tokens[paramIndex++];
                    try
                    {
                        // внутри setMode для 'l' уже stoi
                        channel.setMode('l', true, limitStr);
                        appliedModeFlags << "+l";
                        appliedParams.push_back(limitStr);
                    }
                    catch (...)
                    {
                        // Ошибка: параметр не число
                        std::string reply =
                            "461 MODE l :Invalid limit parameter\r\n";
                        send(fd, reply.c_str(), reply.size(), 0);
                        return;
                    }
                }
                else
                {
                    // снять лимит
                    channel.setMode('l', false);
                    appliedModeFlags << "-l";
                }
            }
            break;

            case 'o':  // дать/снять операторку конкретному нику
            {
                if (paramIndex >= tokens.size())
                {
                    // 461 ERR_NEEDMOREPARAMS
                    std::string reply =
                        "461 MODE :Not enough parameters for +o/-o\r\n";
                    send(fd, reply.c_str(), reply.size(), 0);
                    return;
                }
                std::string targetNick = tokens[paramIndex++];
                int         targetFd   = -1;
                // Ищем по нику
                for (auto& kv : server->getClients())
                {
                    if (kv.second->getNickname() == targetNick)
                    {
                        targetFd = kv.first;
                        break;
                    }
                }
                if (targetFd == -1)
                {
                    // 401 ERR_NOSUCHNICK
                    std::string reply =
                        "401 " + targetNick + " :No such nick\r\n";
                    send(fd, reply.c_str(), reply.size(), 0);
                    // не прерываем всю установку, а выходим,
                    // но в классическом IRC обычно прерывается обработка
                    return;
                }

                if (plus)
                {
                    channel.addOperator(targetFd);
                    appliedModeFlags << "+o";
                    appliedParams.push_back(targetNick);
                }
                else
                {
                    // Снимать оп только если есть кто-то ещё в операторах,
                    // чтобы канал не остался совсем без операторов
                    if (channel.isOperator(targetFd))
                    {
                        // Считаем, сколько операторов
                        int opCount = 0;
                        for (int cfd : channel.getClients())
                        {
                            if (channel.isOperator(cfd)) opCount++;
                        }
                        if (opCount > 1)
                        {
                            channel.removeOperator(targetFd);
                            appliedModeFlags << "-o";
                            appliedParams.push_back(targetNick);
                        }
                        else
                        {
                            // не даём снести последнюю операторку
                            // 482 (re-используем тот же код,
                            // хотя смысл немного иной)
                            std::string reply =
                                "482 " + channelName +
                                " :Cannot remove the last operator\r\n";
                            send(fd, reply.c_str(), reply.size(), 0);
                            return;
                        }
                    }
                    // Если targetFd не оператор, просто игнорируем
                }
            }
            break;

            default:
            {
                // 472 ERR_UNKNOWNMODE
                // "472 <nick> <char> :is unknown mode char to me"
                std::string reply =
                    "472 " + server->getClients()[fd]->getNickname() + " " + c +
                    " :is unknown mode char to me\r\n";
                send(fd, reply.c_str(), reply.size(), 0);
                // Обычно IRC не прерывает полностью обработку,
                // но мы можем прервать, если хотим.
                return;
            }
        }
    }

    // Если ничего не изменилось, можно выйти. Но обычно нужно оповестить,
    // что какие-то режимы точно изменились. Проверяем,
    // вдруг пользователь написал бессмысленное "+", без символов.
    std::string changedModes = appliedModeFlags.str();
    if (changedModes.empty())
    {
        // Ничего не изменилось
        return;
    }

    // 8. Формируем сообщение для рассылки всем в канале.
    // ":<ник> MODE <канал> <список режимов> [параметры]"
    std::ostringstream broadcast;
    broadcast << ":" << server->getClients()[fd]->getNickname() << " MODE "
              << channelName << " " << changedModes;

    for (size_t i = 0; i < appliedParams.size(); ++i)
    {
        broadcast << " " << appliedParams[i];
    }
    broadcast << "\r\n";

    std::string modeMsg = broadcast.str();

    // 9. Шлём всем участникам канала
    for (int memberFd : channel.getClients())
    {
        send(memberFd, modeMsg.c_str(), modeMsg.size(), 0);
    }
}
