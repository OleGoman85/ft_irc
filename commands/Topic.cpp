#include "Topic.hpp"

#include <sys/socket.h>

#include <string>

#include "../include/Server.hpp"

/**
 * @brief Handles the TOPIC command from a client.
 *
 * The TOPIC command is used to either view or change the topic of a channel.
 * If a colon (':') is present in the command following the channel name, the
 * command is interpreted as a request to change the topic. Otherwise, it is
 * assumed that the client wants to view the current topic.
 *
 * When changing the topic, if the channel is set to topic-restricted mode, the
 * client must be an operator. The updated topic is then broadcast to all
 * members of the channel. If viewing the topic, the current topic (if set) is
 * sent back to the client.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the TOPIC command.
 * @param tokens A vector of strings containing the tokenized command arguments.
 *               Expected tokens are: "TOPIC", <channel>, and optionally a new
 * topic (preceded by a colon).
 * @param command The complete command string as received from the client.
 */
void handleTopicCommand(Server* server, int fd,
                        const std::vector<std::string>& tokens,
                        const std::string&              command)
{
    // Verify that the client is fully registered.
    if (server->getClients()[fd]->authState != AUTH_REGISTERED)
    {
        std::string reply = "451 :You have not registered\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Ensure that at least the channel name is provided.
    if (tokens.size() < 2)
    {
        std::string reply = "461 TOPIC :Not enough parameters\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Extract the channel name from the tokens.
    std::string channelName = tokens[1];

    // Look for the channel in the server's channel map.
    auto it = server->getChannels().find(channelName);
    if (it == server->getChannels().end())
    {
        std::string reply = "403 " + channelName + " :No such channel\r\n";
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }

    // Check if a new topic is specified in the command (i.e., there is a
    // colon).
    size_t topicPos = command.find(':');
    if (topicPos != std::string::npos)
    {
        // Skip the colon.
        topicPos++;
        // Extract the new topic.
        std::string newTopic = command.substr(topicPos);

        // If the channel is topic-restricted, verify that the client is an
        // operator.
        if (it->second.isTopicRestricted() && !it->second.isOperator(fd))
        {
            std::string reply =
                "482 " + channelName + " :You're not channel operator\r\n";
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        // Set the new topic for the channel.
        it->second.setTopic(newTopic);
        // Construct a topic change notification message.
        std::string topicMsg = ":" + server->getClients()[fd]->getNickname() +
                               " TOPIC " + channelName + " :" + newTopic +
                               "\r\n";
        // Send the updated topic to all members of the channel.
        for (int cli_fd : it->second.getClients())
        {
            send(cli_fd, topicMsg.c_str(), topicMsg.size(), 0);
        }
    }
    else
    {
        // If no new topic is provided, the client is requesting to view the
        // current topic.
        std::string currentTopic = it->second.getTopic();
        std::string topicReply;
        if (currentTopic.empty())
            topicReply = "331 " + channelName + " :No topic is set\r\n";
        else
            topicReply = "332 " + channelName + " :" + currentTopic + "\r\n";
        send(fd, topicReply.c_str(), topicReply.size(), 0);
    }
}
