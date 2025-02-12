/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aarbenin <aarbenin@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 11:30:54 by ogoman            #+#    #+#             */
/*   Updated: 2025/02/12 13:35:37 by aarbenin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>  // std::transform

#include "../include/Replies.hpp"
#include "../include/Server.hpp"

static void broadcastNickChange(Server* server, Client* client, 
                                const std::string& oldNick, 
                                const std::string& newNick)
{

    std::string message =
        ":" + oldNick + "!" 
        + client->getUsername() + "@" + client->getHost() 
        + " NICK :" + newNick + "\r\n";

    int fd = client->getFd();

    send(fd, message.c_str(), message.size(), 0);

    for (std::map<std::string, Channel>::iterator it = server->getChannels().begin();
         it != server->getChannels().end(); ++it)
    {
        Channel& chan = it->second;
        if (chan.hasClient(fd))
        {
            const std::vector<int>& chanClients = chan.getClients();
            for (size_t i = 0; i < chanClients.size(); i++)
            {
                int otherFd = chanClients[i];
                send(otherFd, message.c_str(), message.size(), 0);
            }
        }
    }
}


void handleNickCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& /*command*/)
{
    if (tokens.size() < 2)
    {
        // 431 ERR_NONICKNAMEGIVEN
        std::string reply = "431 :No nickname given\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    std::string newNick = tokens[1];

    for (const auto& pair : server->getClients())
    {
        if (pair.first != fd && pair.second->getNickname() == newNick)
        {
            // 433 ERR_NICKNAMEINUSE
            std::string reply =
                "433 * " + newNick + " :Nickname is already in use\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
    }

    Client* client = server->getClients()[fd].get();
    if (!client) return;

    std::string oldNick = client->getNickname();

    client->setNickname(newNick);

    AuthState& st = client->authState;

    if (st == NOT_REGISTERED)
    {
        st = WAITING_FOR_USER;
        return;
    }

    if (st == WAITING_FOR_NICK)
    {
        st = WAITING_FOR_USER;
        if (!client->getUsername().empty())
        {
            st = AUTH_REGISTERED;
            sendWelcome(server, fd);
        }
        return;
    }

    if (st == WAITING_FOR_USER)
    {
        if (!client->getUsername().empty())
        {
            st = AUTH_REGISTERED;
            sendWelcome(server, fd);
        }
        return;
    }

    if (st == AUTH_REGISTERED)
    {
        if (oldNick != newNick && !oldNick.empty())
        {
            broadcastNickChange(server, client, oldNick, newNick);
        }
    }
}
