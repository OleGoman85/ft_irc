#include "Replies.hpp"
#include <sys/socket.h>
#include <string>
#include "../include/Client.hpp"
#include "../include/Server.hpp"

/**
 * @brief Sends the welcome message sequence to a newly registered client.
 *
 * This function sends a series of numeric replies to the client, confirming
 * their successful connection to the IRC server. The messages include:
 *  - 001: RPL_WELCOME - A greeting message.
 *  - 002: RPL_YOURHOST - Information about the server and its version.
 *  - 003: RPL_CREATED - A message indicating when the server was created.
 *  - 004: RPL_MYINFO - Server details including supported user modes.
 *
 * @param server Pointer to the Server instance.
 * @param fd The file descriptor of the client receiving the welcome message.
 */
void sendWelcome(Server* server, int fd)
{
    // Retrieve client and server details.
    Client*     client = server->getClients()[fd].get();
    std::string nick   = client->getNickname();
    std::string srv    = server->getServerName();

    // 001: RPL_WELCOME - Confirms successful connection and registration.
    std::string rpl1 = ":" + srv + " 001 " + nick + " :Welcome to " + srv +
                       ", " + nick + "!\r\n";

    // 002: RPL_YOURHOST - Displays server host and version.
    std::string rpl2 = ":" + srv + " 002 " + nick + " :Your host is " + srv +
                       ", running version 1.0\r\n";

    // 003: RPL_CREATED - Indicates when the server was initialized.
    std::string rpl3 =
        ":" + srv + " 003 " + nick + " :This server was created just now\r\n";

    // 004: RPL_MYINFO - Provides server details and supported user modes.
    std::string rpl4 =
        ":" + srv + " 004 " + nick + " " + srv + " 1.0 iwtov\r\n";

    // Send the responses to the client.
    send(fd, rpl1.c_str(), rpl1.size(), 0);
    send(fd, rpl2.c_str(), rpl2.size(), 0);
    send(fd, rpl3.c_str(), rpl3.size(), 0);
    send(fd, rpl4.c_str(), rpl4.size(), 0);
}
