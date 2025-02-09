#include "FileCommand.hpp"
#include "../include/Utils.hpp"
#include "../include/Client.hpp"
#include "../include/FileTransfer.hpp"
#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <stdexcept>

/**
 * @brief Simple base64 decoder for demonstration. 
 *        No external library is used due to the project constraints.
 */
static std::vector<char> base64Decode(const std::string &in);

/**
 * @brief Creates a unique key for identifying a file transfer session.
 *
 * Key format: "<senderFd>_<filename>"
 */
static std::string makeTransferKey(int senderFd,
                                   const std::string &filename)
{
    std::stringstream ss;
    ss << senderFd << "_" << filename;
    return ss.str();
}

/**
 * @brief Main entry point for handling FILE commands.
 *
 * Subcommands:
 *   - FILE SEND <nickname> <filename> <filesize>
 *   - FILE DATA <filename> <base64_chunk>
 *   - FILE END  <filename>
 */
void handleFileCommand(Server* server,
                       int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& fullCommand)
{
    (void)fullCommand; 
    if (tokens.size() < 2) {
        std::string err = "461 FILE :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    // Normalize subcommand to uppercase
    std::string subcmd = tokens[1];
    std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::toupper);

    // ----------------------
    // FILE SEND ...
    // ----------------------
    if (subcmd == "SEND") {
        // Format: FILE SEND <nickname> <filename> <filesize>
        // tokens: [0]="FILE" [1]="SEND"
        //         [2]=nickname [3]=filename [4]=filesize
        if (tokens.size() < 5) {
            std::string err = "461 FILE SEND :Not enough parameters\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        std::string targetNick  = tokens[2];
        std::string filename    = tokens[3];
        std::string filesizeStr = tokens[4];

        size_t filesize = 0;
        try {
            filesize = static_cast<size_t>(std::stoul(filesizeStr));
        } catch (...) {
            std::string err = "461 FILE SEND :Invalid filesize\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        // Look for receiver by nickname
        int receiverFd = -1;
        for (std::map<int, std::unique_ptr<Client> >::iterator it =
                 server->getClients().begin();
             it != server->getClients().end(); ++it)
        {
            if (it->second->getNickname() == targetNick) {
                receiverFd = it->first;
                break;
            }
        }

        if (receiverFd == -1) {
            std::string err = "401 " + targetNick + " :No such nick\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        // Build transfer key
        std::string key = makeTransferKey(fd, filename);

        // If a transfer with this key already exists, we can either overwrite or reject
        if (server->getFileTransfers().count(key) != 0) {
            server->getFileTransfers().erase(key);
        }

        // Create a new FileTransfer
        FileTransfer ft(fd, receiverFd, filename, filesize);
        server->getFileTransfers().insert(std::make_pair(key, ft));

        // Notify the sender
        {
            std::string msg = "NOTICE " + targetNick +
                              " :Ready to receive file '" + filename +
                              "' (" + filesizeStr + " bytes)\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
        }

        // Notify the receiver
        {
            std::string msg = ":" + server->getClients()[fd]->getNickname() +
                              " PRIVMSG " + targetNick +
                              " :Incoming file: " + filename +
                              " (" + filesizeStr +
                              " bytes). Use FILE commands to handle.\r\n";
            send(receiverFd, msg.c_str(), msg.size(), 0);
        }
    }

    // ----------------------
    // FILE DATA ...
    // ----------------------
    else if (subcmd == "DATA") {
        // Format: FILE DATA <filename> <base64_chunk>
        // tokens: [0]="FILE" [1]="DATA"
        //         [2]=filename [3]=base64_chunk
        if (tokens.size() < 4) {
            std::string err = "461 FILE DATA :Not enough parameters\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        std::string filename    = tokens[2];
        std::string base64chunk = tokens[3];
        
        std::string key = makeTransferKey(fd, filename);
        if (server->getFileTransfers().count(key) == 0) {
            std::string err = "400 :No such file transfer session\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }
        
        FileTransfer &ft = server->getFileTransfers()[key];

        // Decode base64
        std::vector<char> decodedData = base64Decode(base64chunk);

        // Append to file buffer
        ft.appendData(decodedData);

        // Optionally notify about progress
        {
            std::ostringstream oss;
            oss << "NOTICE "
                << server->getClients()[fd]->getNickname()
                << " :Uploaded " << ft.getReceivedBytes()
                << "/" << ft.getFilesize() << " bytes of ["
                << ft.getFilename() << "]\r\n";
            std::string msg = oss.str();
            send(fd, msg.c_str(), msg.size(), 0);
        }
    }

    // ----------------------
    // FILE END ...
    // ----------------------
    else if (subcmd == "END") {
        // Format: FILE END <filename>
        if (tokens.size() < 3) {
            std::string err = "461 FILE END :Not enough parameters\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }
        std::string filename = tokens[2];

        std::string key = makeTransferKey(fd, filename);
        if (server->getFileTransfers().count(key) == 0) {
            std::string err = "400 :No such file transfer session\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        FileTransfer &ft = server->getFileTransfers()[key];
        bool complete = ft.isComplete();
        int receiverFd = ft.getReceiverFd();

        // Notify sender
        if (!complete) {
            std::string msgSender = "NOTICE " +
                server->getClients()[fd]->getNickname() +
                " :File transfer ended, but file is incomplete (" +
                std::to_string(ft.getReceivedBytes()) + "/" +
                std::to_string(ft.getFilesize()) + ")\r\n";
            send(fd, msgSender.c_str(), msgSender.size(), 0);
        } else {
            std::string msgSender = "NOTICE " +
                server->getClients()[fd]->getNickname() +
                " :File transfer completed (" + ft.getFilename() + ")\r\n";
            send(fd, msgSender.c_str(), msgSender.size(), 0);
        }

        // Send the file to the receiver (for demonstration).
        {
            const std::vector<char>& fileBuf = ft.getFileBuffer();

            // A short info message to the receiver
            std::ostringstream oss;
            oss << "NOTICE "
                << server->getClients()[receiverFd]->getNickname()
                << " :You have received file ["
                << ft.getFilename()
                << "] with size " << fileBuf.size() << " bytes\r\n";
            std::string infoMsg = oss.str();
            send(receiverFd, infoMsg.c_str(), infoMsg.size(), 0);

            // In a real scenario, we'd prefer DCC or other method for big files.
            // Here, we just do a direct send of raw data (which may look like garbage
            // in an IRC client). One could base64 it back, or do chunked sends.
            if (!fileBuf.empty()) {
                send(receiverFd, &fileBuf[0], fileBuf.size(), 0);
            }
        }

        // Remove the transfer session
        server->getFileTransfers().erase(key);
    }

    // ----------------------
    // Unknown subcommand
    // ----------------------
    else {
        std::string err = "400 :Unknown FILE subcommand\r\n";
        send(fd, err.c_str(), err.size(), 0);
    }
}

/**
 * @brief Minimal base64 decode logic. 
 *        This ignores invalid characters outside base64 and '=' for padding.
 */
static const std::string base64Chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool isBase64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::vector<char> base64Decode(const std::string &encodedString)
{
    int inLen = static_cast<int>(encodedString.size());
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char charArray4[4], charArray3[3];
    std::vector<char> ret;

    while (inLen-- &&
           (encodedString[in_] != '=') &&
            isBase64(encodedString[in_]))
    {
        charArray4[i++] = encodedString[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] =
                    static_cast<unsigned char>(base64Chars.find(charArray4[i]));
            }

            charArray3[0] = (char)((charArray4[0] << 2) +
                            ((charArray4[1] & 0x30) >> 4));
            charArray3[1] = (char)(((charArray4[1] & 0x0F) << 4) +
                            ((charArray4[2] & 0x3C) >> 2));
            charArray3[2] = (char)(((charArray4[2] & 0x03) << 6) +
                             charArray4[3]);

            for (i = 0; i < 3; i++) {
                ret.push_back(charArray3[i]);
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            charArray4[j] = 0;
        }
        for (j = 0; j < 4; j++) {
            charArray4[j] =
                static_cast<unsigned char>(base64Chars.find(charArray4[j]));
        }

        charArray3[0] = (char)((charArray4[0] << 2) +
                        ((charArray4[1] & 0x30) >> 4));
        charArray3[1] = (char)(((charArray4[1] & 0x0F) << 4) +
                        ((charArray4[2] & 0x3C) >> 2));
        charArray3[2] = (char)(((charArray4[2] & 0x03) << 6) +
                         charArray4[3]);

        for (j = 0; j < (i - 1); j++) {
            ret.push_back(charArray3[j]);
        }
    }

    return ret;
}
