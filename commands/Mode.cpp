#include <cctype>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "../include/Channel.hpp"
#include "../include/Server.hpp"

struct ModeChange
{
    bool        add;    // true для '+', false для '-'
    char        mode;   // символ режима
    std::string param;  // параметр, если требуется
};

void handleModeCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string&              rawCommand)
{
    (void)rawCommand;  // не используется прямо сейчас

    // 1. Проверка регистрации
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 2. Проверка, что указали хотя бы MODE #channel
    if (tokens.size() < 2)
    {
        std::string reply = "461 MODE :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string channelName = tokens[1];
    auto        it          = server->getChannels().find(channelName);
    if (it == server->getChannels().end())
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    Channel& channel = it->second;

    // 3. Если только "MODE #channel", выводим текущие режимы
    if (tokens.size() == 2)
    {
        std::string modes;
        if (channel.isInviteOnly()) modes += "i";
        if (channel.isTopicRestricted()) modes += "t";
        if (channel.hasMode('k')) modes += "k";
        if (channel.hasMode('l')) modes += "l";

        modes = modes.empty() ? "+" : "+" + modes;

        std::string reply = "324 " + server->getClients()[fd]->getNickname() +
                            " " + channelName + " " + modes;

        if (channel.hasMode('k')) reply += " " + channel.getChannelKey();
        if (channel.hasMode('l'))
            reply += " " + std::to_string(channel.getUserLimit());

        reply += "\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 4. Только операторы могут менять режимы
    if (!channel.isOperator(fd))
    {
        std::string reply =
            "482 " + channelName + " :You're not a channel operator\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // 5. Валидация строки режимов
    std::string modeStr = tokens[2];
    if (modeStr.empty() || (modeStr[0] != '+' && modeStr[0] != '-'))
    {
        std::string err = "472 " + server->getClients()[fd]->getNickname() +
                          " :Invalid mode string\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    size_t paramIdx    = 3;  // Индекс следующего параметра в tokens
    bool   currentSign = (modeStr[0] == '+');
    std::vector<ModeChange> changes;

    // 6. Разбор строки флагов
    for (size_t i = 0; i < modeStr.size(); ++i)
    {
        char c = modeStr[i];
        if (c == '+')
        {
            currentSign = true;
            continue;
        }
        if (c == '-')
        {
            currentSign = false;
            continue;
        }

        ModeChange change;
        change.add  = currentSign;
        change.mode = c;

        switch (c)
        {
            case 'i':
            case 't':
            {
                channel.setMode(c, currentSign);
            }
            break;

            case 'k':
            {
                if (currentSign)
                {
                    if (paramIdx >= tokens.size())
                    {
                        std::string err =
                            "461 MODE :Not enough parameters for +k\r\n";
                        send(fd, err.c_str(), err.size(), 0);
                        return;
                    }
                    std::string key = tokens[paramIdx++];
                    channel.setMode('k', true, key);
                    change.param = key;
                }
                else
                {
                    channel.setMode('k', false);
                }
            }
            break;

            case 'l':
            {
                if (currentSign)
                {
                    if (paramIdx >= tokens.size())
                    {
                        std::string err =
                            "461 MODE :Not enough parameters for +l\r\n";
                        send(fd, err.c_str(), err.size(), 0);
                        return;
                    }
                    std::string limitStr = tokens[paramIdx++];
                    try
                    {
                        int limit = std::stoi(limitStr);
                        if (limit <= 0)
                        {
                            std::string err =
                                "461 MODE l :Invalid limit parameter\r\n";
                            send(fd, err.c_str(), err.size(), 0);
                            return;
                        }
                        channel.setMode('l', true, limitStr);
                        change.param = limitStr;
                    }
                    catch (const std::exception&)
                    {
                        std::string err =
                            "461 MODE l :Invalid limit parameter\r\n";
                        send(fd, err.c_str(), err.size(), 0);
                        return;
                    }
                }
                else
                {
                    channel.setMode('l', false);
                }
            }
            break;

            case 'o':
            {
                if (paramIdx >= tokens.size())
                {
                    std::string err =
                        "461 MODE :Not enough parameters for +o/-o\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    return;
                }
                std::string targetNick = tokens[paramIdx++];
                int         targetFd   = -1;
                for (auto& [cliFd, cliPtr] : server->getClients())
                {
                    if (cliPtr->getNickname() == targetNick)
                    {
                        targetFd = cliFd;
                        break;
                    }
                }
                if (targetFd == -1)
                {
                    std::string err =
                        "401 " + targetNick + " :No such nick\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    return;
                }
                if (!channel.hasClient(targetFd))
                {
                    std::string err = "441 " + targetNick + " " + channelName +
                                      " :They aren't on that channel\r\n";
                    send(fd, err.c_str(), err.size(), 0);
                    return;
                }

                if (currentSign)
                {
                    channel.addOperator(targetFd);
                }
                else
                {
                    if (channel.isOperator(targetFd))
                    {
                        int opCount = 0;
                        for (int cfd : channel.getClients())
                        {
                            if (channel.isOperator(cfd)) opCount++;
                        }
                        if (opCount > 1)
                        {
                            channel.removeOperator(targetFd);
                        }
                        else
                        {
                            std::string err =
                                "482 " + channelName +
                                " :Cannot remove the last operator\r\n";
                            send(fd, err.c_str(), err.size(), 0);
                            return;
                        }
                    }
                    // Если targetFd не оператор, можно игнорировать
                }
                change.param = targetNick;
            }
            break;

            default:
            {
                std::string err = "472 " +
                                  server->getClients()[fd]->getNickname() +
                                  " " + c + " :is unknown mode char to me\r\n";
                send(fd, err.c_str(), err.size(), 0);
                continue;
            }
        }
        changes.push_back(change);
    }

    // 7. Формирование итоговой строки с группировкой по знаку
    std::ostringstream       modeStream;
    std::vector<std::string> modeParams;
    if (!changes.empty())
    {
        // Начинаем с первого знака
        modeStream << (changes[0].add ? "+" : "-");
        char currentGroupSign = changes[0].add ? '+' : '-';

        for (size_t i = 0; i < changes.size(); ++i)
        {
            if ((changes[i].add ? '+' : '-') != currentGroupSign)
            {
                // Если знак изменился, начинаем новую группу (с пробелом)
                modeStream << " " << (changes[i].add ? "+" : "-");
                currentGroupSign = changes[i].add ? '+' : '-';
            }
            modeStream << changes[i].mode;
            if (!changes[i].param.empty())
                modeParams.push_back(changes[i].param);
        }
    }
    std::string finalModeStr = modeStream.str();
    if (finalModeStr.empty()) return;

    // 8. Формирование сообщения для рассылки
    std::ostringstream broadcast;
    broadcast << ":" << server->getClients()[fd]->getNickname() << " MODE "
              << channelName << " " << finalModeStr;
    for (const std::string& p : modeParams)
    {
        broadcast << " " << p;
    }
    broadcast << "\r\n";
    std::string msg = broadcast.str();

    // Рассылаем сообщение всем участникам канала
    for (int memberFd : channel.getClients())
    {
        send(memberFd, msg.c_str(), msg.size(), 0);
    }
}
