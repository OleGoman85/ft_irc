/************************************************************
 * FileCommand.cpp – Example implementation of the FILE command
 * (SEND/DATA/END) with correct IRC message formatting.
 ************************************************************/

#include "FileCommand.hpp"
#include <algorithm>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

#include "../include/Client.hpp"
#include "../include/FileTransfer.hpp"
#include "../include/Utils.hpp"

/**
 * @brief Minimal base64 decoder without external libraries.
 */
static std::vector<char> base64Decode(const std::string& in);

/**
 * @brief Generates a unique key in the format "fd_filename" for storing a FileTransfer.
 */
static std::string makeTransferKey(int senderFd, const std::string& filename)
{
    std::stringstream ss;
    ss << senderFd << "_" << filename;
    return ss.str();
}

/* ------------------------------------------------------------------
 * Helper functions for each subcommand
 * ------------------------------------------------------------------ */

/**
 * @brief Handles the FILE SEND command: FILE SEND <nickname> <filename> <filesize>
 */
static void handleFileSend(Server* server, int fd,
    const std::vector<std::string>& tokens)
{
    if (tokens.size() < 5) {
        std::string err = "461 FILE SEND :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    std::string targetNick = tokens[2];
    std::string filename = tokens[3];
    if (!filename.empty() && filename[0] == ':')
        filename.erase(0, 1);

    std::string filesizeStr = tokens[4];
    size_t filesize = 0;
    try {
        filesize = static_cast<size_t>(std::stoul(filesizeStr));
    } catch (...) {
        std::string err = "461 FILE SEND :Invalid filesize\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    int receiverFd = -1;
    for (std::map<int, std::unique_ptr<Client>>::iterator it = server->getClients().begin();
        it != server->getClients().end(); ++it) {
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

    std::string key = makeTransferKey(fd, filename);
    if (server->getFileTransfers().count(key) != 0) {
        server->getFileTransfers().erase(key);
    }

    FileTransfer ft(fd, receiverFd, filename, filesize);
    server->getFileTransfers().insert(std::make_pair(key, ft));

    {
        std::string msg = ":" + server->getServerName() + " NOTICE " + server->getClients()[fd]->getNickname() + " :Ready to receive file '" + filename + "' (" + filesizeStr + " bytes)\r\n";
        send(fd, msg.c_str(), msg.size(), 0);
    }

    {
        std::string msg = ":" + server->getServerName() + " NOTICE " + targetNick + " :Incoming file: " + filename + " (" + filesizeStr + " bytes).\r\n";
        send(receiverFd, msg.c_str(), msg.size(), 0);
    }
}

/**
 * @brief Handles the FILE DATA command: FILE DATA <filename> <base64_chunk...>
 */
static void handleFileData(Server* server, int fd,
    const std::vector<std::string>& tokens)
{
    if (tokens.size() < 3) {
        std::string err = "461 FILE DATA :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    std::string filename = tokens[2];
    if (!filename.empty() && filename[0] == ':')
        filename.erase(0, 1);

    std::string base64chunk;
    if (tokens.size() > 3) {
        for (size_t i = 3; i < tokens.size(); i++) {
            if (!base64chunk.empty())
                base64chunk += " ";
            std::string part = tokens[i];
            if (!part.empty() && part[0] == ':')
                part.erase(0, 1);
            base64chunk += part;
        }
    }

    std::string key = makeTransferKey(fd, filename);
    if (server->getFileTransfers().count(key) == 0) {
        std::string err = "400 :No such file transfer session\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }
    FileTransfer& ft = server->getFileTransfers()[key];

    std::vector<char> decodedData = base64Decode(base64chunk);
    ft.appendData(decodedData);

    {
        std::ostringstream oss;
        oss << ":" << server->getServerName() << " NOTICE "
            << server->getClients()[fd]->getNickname() << " :Uploaded "
            << ft.getReceivedBytes() << "/" << ft.getFilesize()
            << " bytes of [" << ft.getFilename() << "]\r\n";
        std::string msg = oss.str();
        send(fd, msg.c_str(), msg.size(), 0);
    }
}

/**
 * @brief Handles the FILE END command: FILE END <filename>
 */
static void handleFileEnd(Server* server, int fd,
    const std::vector<std::string>& tokens)
{
    if (tokens.size() < 3) {
        std::string err = "461 FILE END :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    std::string filename = tokens[2];
    if (!filename.empty() && filename[0] == ':')
        filename.erase(0, 1);

    std::string key = makeTransferKey(fd, filename);
    if (server->getFileTransfers().count(key) == 0) {
        std::string err = "400 :No such file transfer session\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    FileTransfer& ft = server->getFileTransfers()[key];
    bool complete = ft.isComplete();
    int receiverFd = ft.getReceiverFd();

    if (!complete) {
        std::string msgSender = ":" + server->getServerName() + " NOTICE " + server->getClients()[fd]->getNickname() + " :File transfer ended, but file is incomplete (" + std::to_string(ft.getReceivedBytes()) + "/" + std::to_string(ft.getFilesize()) + ")\r\n";
        send(fd, msgSender.c_str(), msgSender.size(), 0);
    } else {
        std::string msgSender = ":" + server->getServerName() + " NOTICE " + server->getClients()[fd]->getNickname() + " :File transfer completed (" + ft.getFilename() + ")\r\n";
        send(fd, msgSender.c_str(), msgSender.size(), 0);
    }

    {
        std::ostringstream oss;
        oss << ":" << server->getServerName() << " NOTICE "
            << server->getClients()[receiverFd]->getNickname()
            << " :You have received file [" << ft.getFilename()
            << "] with size " << ft.getFileBuffer().size() << " bytes\r\n";
        std::string infoMsg = oss.str();
        send(receiverFd, infoMsg.c_str(), infoMsg.size(), 0);
    }

    {
        const std::vector<char>& fileBuf = ft.getFileBuffer();
        if (!fileBuf.empty()) {
            send(receiverFd, &fileBuf[0], fileBuf.size(), 0);
        }
    }

    server->getFileTransfers().erase(key);
}

/**
 * @brief Main handler for the FILE command.
 */
void handleFileCommand(Server* server, int fd,
    const std::vector<std::string>& tokens,
    const std::string& /*fullCommand*/)
{
    if (tokens.size() < 2) {
        std::string err = "461 FILE :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    std::string subcmd = tokens[1];
    std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::toupper);

    if (subcmd == "SEND") {
        handleFileSend(server, fd, tokens);
    } else if (subcmd == "DATA") {
        handleFileData(server, fd, tokens);
    } else if (subcmd == "END") {
        handleFileEnd(server, fd, tokens);
    } else {
        std::string err = "400 :Unknown FILE subcommand\r\n";
        send(fd, err.c_str(), err.size(), 0);
    }
}

/* ------------------------------------------------------------------
 * Minimal base64Decode implementation
 * ------------------------------------------------------------------ */
static const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "0123456789+/";

static inline bool isBase64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::vector<char> base64Decode(const std::string& encodedString)
{
    int inLen = static_cast<int>(encodedString.size());
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char charArray4[4], charArray3[3];
    std::vector<char> ret;

    while (inLen-- && (encodedString[in_] != '=') && isBase64(encodedString[in_])) {
        charArray4[i++] = encodedString[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] = static_cast<unsigned char>(base64Chars.find(charArray4[i]));
            }

            charArray3[0] = (char)((charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4));
            charArray3[1] = (char)(((charArray4[1] & 0x0F) << 4) + ((charArray4[2] & 0x3C) >> 2));
            charArray3[2] = (char)(((charArray4[2] & 0x03) << 6) + charArray4[3]);

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
            charArray4[j] = static_cast<unsigned char>(base64Chars.find(charArray4[j]));
        }

        charArray3[0] = (char)((charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4));
        charArray3[1] = (char)(((charArray4[1] & 0x0F) << 4) + ((charArray4[2] & 0x3C) >> 2));
        charArray3[2] = (char)(((charArray4[2] & 0x03) << 6) + charArray4[3]);

        for (j = 0; j < (i - 1); j++) {
            ret.push_back(charArray3[j]);
        }
    }

    return ret;
}
