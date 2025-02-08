/**
 * @file BotCommand.cpp
 * @brief Implementation of the IRC bot command handling.
 */

#include "BotCommand.hpp"
#include <algorithm>   // for std::transform
#include <cstdlib>     // for std::rand, std::srand
#include <ctime>       // for std::time
#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"

/**
 * @brief Sends a message to a client socket, adding "\r\n" at the end.
 *
 * @param fd  The client socket file descriptor.
 * @param msg The message to send (without "\r\n").
 */
static void sendToClient(int fd, const std::string& msg)
{
    std::string withCrLf = msg + "\r\n";
    send(fd, withCrLf.c_str(), withCrLf.size(), 0);
}

/**
 * @brief Sends a message to all members of a channel, emulating a bot user.
 *        The bot has a fixed nickname "AwesomeBot".
 *
 * @param server  Pointer to the Server instance (currently unused).
 * @param channel Reference to the Channel object where the message is sent.
 * @param message The message content to broadcast in the channel.
 */
static void sendToChannelAsBot(Server* server, Channel& channel, const std::string& message)
{
    (void)server; // Unused parameter

    // Construct an IRC-style prefix: ":AwesomeBot!BotUser@irc.local PRIVMSG #channel :Message"
    std::string prefix = ":AwesomeBot!BotUser@irc.local PRIVMSG " + channel.getName() + " :";
    std::string fullMsg = prefix + message + "\r\n";

    // Send the message to each channel member
    const std::vector<int>& members = channel.getClients();
    for (size_t i = 0; i < members.size(); ++i)
    {
        int memberFd = members[i];
        send(memberFd, fullMsg.c_str(), fullMsg.size(), 0);
    }
}

/**
 * @brief Generates a random 8-Ball style answer.
 * 
 * @return A random yes/no/maybe style message.
 */
static std::string getRandom8BallAnswer()
{
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(static_cast<unsigned int>(std::time(NULL)));
        seeded = true;
    }

    static const char* answers[] = {
        "Yes!",
        "No!",
        "Maybe...",
        "Certainly yes",
        "Ask again later",
        "Definitely no!",
        "Chances are low, but not zero",
        "Ask the neighbor's cat"
    };

    int size = static_cast<int>(sizeof(answers) / sizeof(answers[0]));
    int randomIndex = std::rand() % size;
    return answers[randomIndex];
}

/**
 * @brief Handles the "BOT" command from a client. 
 *
 * The bot supports the following subcommands:
 * - BOT JOIN #channel
 * - BOT LEAVE #channel
 * - BOT SAY #channel your message
 * - BOT 8BALL any question
 *
 * @param server      Pointer to the Server instance.
 * @param fd          File descriptor of the client issuing the command.
 * @param tokens      Tokenized command parts.
 * @param fullCommand The full command string (unused).
 */
void handleBotCommand(Server* server, int fd,
                      const std::vector<std::string>& tokens,
                      const std::string& fullCommand)
{
    (void)fullCommand; // Unused parameter

    // Must have at least "BOT <subcommand>"
    if (tokens.size() < 2)
    {
        sendToClient(fd, "461 BOT :Not enough parameters");
        return;
    }

    // Transform subcommand to uppercase for case-insensitive matching
    std::string subCommand = tokens[1];
    std::transform(subCommand.begin(), subCommand.end(), subCommand.begin(), ::toupper);

    if (subCommand == "JOIN")
    {
        if (tokens.size() < 3)
        {
            sendToClient(fd, "461 BOT JOIN :Not enough parameters");
            return;
        }
        std::string channelName = tokens[2];

        // Create the channel if it does not exist
        if (server->getChannels().find(channelName) == server->getChannels().end())
        {
            server->getChannels().insert(std::make_pair(channelName, Channel(channelName)));
        }

        Channel& channelRef = server->getChannels()[channelName];

        sendToClient(fd, "Bot joined channel " + channelName);
        sendToChannelAsBot(server, channelRef, "Hello, everyone! I've joined " + channelName);
    }
    else if (subCommand == "LEAVE")
    {
        if (tokens.size() < 3)
        {
            sendToClient(fd, "461 BOT LEAVE :Not enough parameters");
            return;
        }
        std::string channelName = tokens[2];

        if (server->getChannels().find(channelName) == server->getChannels().end())
        {
            sendToClient(fd, "403 " + channelName + " :No such channel");
            return;
        }
        Channel& channelRef = server->getChannels()[channelName];

        sendToChannelAsBot(server, channelRef, "Goodbye, friends! I've left " + channelName);
        sendToClient(fd, "Bot left channel " + channelName);
    }
    else if (subCommand == "SAY")
    {
        if (tokens.size() < 3)
        {
            sendToClient(fd, "461 BOT SAY :Not enough parameters");
            return;
        }
        std::string channelName = tokens[2];

        if (server->getChannels().find(channelName) == server->getChannels().end())
        {
            sendToClient(fd, "403 " + channelName + " :No such channel");
            return;
        }
        Channel& channelRef = server->getChannels()[channelName];

        if (tokens.size() < 4)
        {
            sendToClient(fd, "461 BOT SAY :No message given");
            return;
        }

        // Combine all remaining tokens into a single message
        std::string message;
        for (size_t i = 3; i < tokens.size(); ++i)
        {
            if (i > 3)
                message += " ";
            message += tokens[i];
        }

        sendToChannelAsBot(server, channelRef, message);
        sendToClient(fd, "Bot message sent to " + channelName);
    }
    else if (subCommand == "8BALL")
    {
        if (tokens.size() < 3)
        {
            sendToClient(fd, "461 BOT 8BALL :Not enough parameters (you must ask a question!)");
            return;
        }

        // Combine all tokens after "8BALL" into one question
        std::string question;
        for (size_t i = 2; i < tokens.size(); ++i)
        {
            if (i > 2)
                question += " ";
            question += tokens[i];
        }

        std::string answer = getRandom8BallAnswer();
        std::string reply = "Magic 8-Ball says: " + answer;
        sendToClient(fd, reply);
    }
    else
    {
        // Unknown subcommand
        sendToClient(fd, "421 BOT " + subCommand + " :Unknown BOT subcommand");
    }
}
