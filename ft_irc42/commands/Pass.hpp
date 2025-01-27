/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pass.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ogoman <ogoman@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 10:43:25 by ogoman            #+#    #+#             */
/*   Updated: 2025/01/16 09:59:19 by ogoman           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PASS_HPP
#define PASS_HPP

#include <string>
#include <vector>

/**
 * @brief Forward declaration of the Server class.
 */
class Server;

/**
 * @brief Handles the PASS command.
 *
 * This function processes the PASS command issued by a client. It verifies the supplied password against the server's password.
 * If the provided password is correct, the client's authentication state is updated (typically to WAITING_FOR_NICK), and a success message is sent.
 * Otherwise, an error message is returned and the client is removed from the server.
 *
 * @param server Pointer to the Server object managing the IRC server.
 * @param fd The file descriptor of the client issuing the PASS command.
 * @param tokens A vector of strings containing the tokenized command arguments (expected format: "PASS" followed by <password>).
 * @param command The complete command string received from the client.
 */
void handlePassCommand(Server* server, int fd, const std::vector<std::string>& tokens, const std::string& command);

#endif // PASS_HPP
