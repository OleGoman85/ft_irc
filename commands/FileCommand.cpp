#include "FileCommand.hpp"
#include "../include/Utils.hpp"
#include "../include/Client.hpp"
#include "../include/FileTransfer.hpp"

#include <iostream>
#include <cerrno>
#include <unistd.h>    // close
#include <algorithm>
#include <sstream>
#include <stdexcept>

// Простейший декодер base64. Можно подключить сторонний кодек, но
// в задании сказано не использовать внешние библиотеки. 
// Для простоты можно реализовать мини-функцию здесь.
// (Или любой мини-парсер base64 на свой вкус.)
static std::vector<char> base64Decode(const std::string &in);

// --------------------------------------------------------------------
// Вспомогательная функция: найти FileTransfer по (fd, filename).
// Мы будем искать в map<string, FileTransfer>, где ключ = уникальный идентификатор
// вида "<senderFd>_<filename>" или "<senderFd>_<receiverFd>_<filename>"
// --------------------------------------------------------------------
static std::string makeTransferKey(int senderFd, const std::string &filename) {
    // Упрощённо: key = "<senderFd>_<filename>"
    // (или можно включить receiverFd, если хотим полную уникальность)
    std::stringstream ss;
    ss << senderFd << "_" << filename;
    return ss.str();
}

void handleFileCommand(Server* server, int fd,
                       const std::vector<std::string>& tokens,
                       const std::string& fullCommand)
{
	(void)fullCommand;
    // Проверка на минимум аргументов:
    //   FILE <subcommand> ...
    if (tokens.size() < 2) {
        std::string err = "461 FILE :Not enough parameters\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return;
    }

    // Подкоманда
    std::string subCmd = tokens[1];
    std::transform(subCmd.begin(), subCmd.end(), subCmd.begin(), ::toupper);

    if (subCmd == "SEND") {
        // Формат: FILE SEND <nickname> <filename> <filesize>
        // tokens: [0]="FILE" [1]="SEND" [2]=nickname [3]=filename [4]=filesize
        if (tokens.size() < 5) {
            std::string err = "461 FILE SEND :Not enough parameters\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        std::string targetNick = tokens[2];
        std::string filename   = tokens[3];
        std::string filesizeStr= tokens[4];
        
        size_t filesize;
        try {
            filesize = static_cast<size_t>(std::stoul(filesizeStr));
        }
        catch (...) {
            std::string err = "461 FILE SEND :Invalid filesize\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        // Проверяем, есть ли такой получатель
        // В Server::_clients хранится map<int, unique_ptr<Client>>
        // Нужно найти среди них клиента с нужным ником
        int receiverFd = -1;
        for (std::map<int, std::unique_ptr<Client> >::iterator it = server->getClients().begin();
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

        // Создаём новый FileTransfer в server->_fileTransfers
        std::string key = makeTransferKey(fd, filename);

        // Если вдруг уже есть трансфер с таким ключом — можно либо перезаписать, либо ругаться
        if (server->getFileTransfers().count(key) != 0) {
            // Удалим старую запись для простоты
            server->getFileTransfers().erase(key);
        }

        FileTransfer ft(fd, receiverFd, filename, filesize);
        server->getFileTransfers().insert(std::make_pair(key, ft));

        // Отправляем отправителю ответ, что можно начинать передачу
        {
            std::string msg = "NOTICE " + targetNick + " :Ready to receive file '" +
                              filename + "' (" + filesizeStr + " bytes)\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
        }

        // Уведомляем получателя, что к нему «стучится» файл
        {
            // В IRC обычно можно прислать PRIVMSG или NOTICE
            // Для наглядности сделаем PRIVMSG:
            std::string msg = ":" + server->getClients()[fd]->getNickname() + 
                              " PRIVMSG " + targetNick + 
                              " :Incoming file: " + filename + 
                              " (" + filesizeStr + " bytes). Use FILE commands to handle.\r\n";
            send(receiverFd, msg.c_str(), msg.size(), 0);
        }
    }
    else if (subCmd == "DATA") {
        // Формат: FILE DATA <filename> <base64_chunk>
        // tokens: [0]="FILE" [1]="DATA" [2]=filename [3]=base64_chunk (всё остальное можно склеивать)
        if (tokens.size() < 4) {
            std::string err = "461 FILE DATA :Not enough parameters\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }

        std::string filename   = tokens[2];
        // Остальные токены после [2] могут относиться к части base64
        // Но в рамках упрощённого варианта предположим,
        // что base64_chunk = tokens[3]. Если там пробелы, их нетривиально парсить —
        // лучше всегда передавать chunk как один параметр без пробелов
        // (настоящий base64 без пробелов).
        std::string base64chunk= tokens[3];

        // Ищем FileTransfer
        std::string key = makeTransferKey(fd, filename);
        if (server->getFileTransfers().count(key) == 0) {
            std::string err = "400 :No such file transfer session\r\n";
            send(fd, err.c_str(), err.size(), 0);
            return;
        }
        
        FileTransfer &ft = server->getFileTransfers()[key];
        
        // Декодируем base64
        std::vector<char> decodedData = base64Decode(base64chunk);

        // Добавляем в буфер
        ft.appendData(decodedData);

        // (опционально) Посылаем отправителю или получателю уведомление о прогрессе
        // Можно только отправителю:
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
    else if (subCmd == "END") {
        // Формат: FILE END <filename>
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

        // Проверяем, дошли ли мы до нужного размера (ft.isComplete())
        // Если не дошли - можно ругаться, либо принять как есть
        bool ok = ft.isComplete();

        int receiverFd = ft.getReceiverFd();

        if (!ok) {
            // Допустим, всё равно завершим трансфер, но уведомим, что файл частичный
            std::string msgSender = "NOTICE " + server->getClients()[fd]->getNickname()
                + " :File transfer ended, but file is incomplete (" 
                + std::to_string(ft.getReceivedBytes())
                + "/" + std::to_string(ft.getFilesize()) + ")\r\n";
            send(fd, msgSender.c_str(), msgSender.size(), 0);
        } else {
            // Успешно
            std::string msgSender = "NOTICE " + server->getClients()[fd]->getNickname()
                + " :File transfer completed (" + ft.getFilename() + ")\r\n";
            send(fd, msgSender.c_str(), msgSender.size(), 0);
        }

        // Теперь пересылаем получателю сам файл — в реальном IRC это делается DCC,
        // но мы по заданию «через сервер».  
        // *ВНИМАНИЕ:* Для больших файлов это может быть очень неэффективно.
        // Здесь сделаем отправку блокирующе (или почасовую в цикле).
        // Чтобы по-простому: возьмём `ft.getFileBuffer()` и вышлем chunk'ом.
        {
            const std::vector<char>& fileBuf = ft.getFileBuffer();

            // Можно какое-то «заголовочное» сообщение
            std::ostringstream oss;
            oss << "NOTICE " 
                << server->getClients()[receiverFd]->getNickname()
                << " :You have received file ["
                << ft.getFilename()
                << "] with size " << fileBuf.size() << " bytes\r\n";
            std::string infoMsg = oss.str();
            send(receiverFd, infoMsg.c_str(), infoMsg.size(), 0);

            // В реальном мире, чтобы не ломать IRC-протокол, такие большие бинарные данные
            // обычно не шлют напрямую. Но, предположим, мы шлём их «как есть» (или в base64).
            // Для примера тут шлём «сырые» байты, что может выглядеть как мусор в клиенте.
            // Лучше было бы либо снова base64-encode + PRIVMSG, либо что-то подобное.
            // Для демонстрации отправим base64:
            //  1) Закодируем fileBuf
            //  2) Отправим одной (или несколькими) PRIVMSG
            //  3) Завершим
            //
            // Но тут для краткости отправим «как есть» одним send'ом. IRC-клиент это увидит
            // как мешанину символов.
            //
            // *Если нужно chunk'ами — придётся делать цикл:
            //   while (not all data sent) { poll(); send(); }
            // Но здесь покажем упрощённо.

            if (!fileBuf.empty()) {
                send(receiverFd, &fileBuf[0], fileBuf.size(), 0);
            }
        }

        // Удаляем запись о трансфере
        server->getFileTransfers().erase(key);
    }
    else {
        // Неизвестный subcommand
        std::string err = "400 :Unknown FILE subcommand\r\n";
        send(fd, err.c_str(), err.size(), 0);
    }
}

// ------------------------------------------------------------
// Простейшая реализация base64 decode (для демонстрации).
// Если в вашей реализации проекта есть готовая, используйте её.
// ------------------------------------------------------------
static const std::string base64Chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool isBase64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::vector<char> base64Decode(const std::string &encodedString) {
    int in_len = static_cast<int>(encodedString.size());
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<char> ret;

    while (in_len-- && ( encodedString[in_] != '=') && isBase64(encodedString[in_])) {
        char_array_4[i++] = encodedString[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = static_cast<unsigned char>(base64Chars.find(char_array_4[i]));

            char_array_3[0] = (char)(( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4));
            char_array_3[1] = (char)(((char_array_4[1] & 0xf ) << 4) + ((char_array_4[2] & 0x3c) >> 2));
            char_array_3[2] = (char)(((char_array_4[2] & 0x3 ) << 6) +   char_array_4[3]        );

            for (i = 0; i < 3; i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;
        for (j = 0; j < 4; j++)
            char_array_4[j] = static_cast<unsigned char>(base64Chars.find(char_array_4[j]));

        char_array_3[0] = (char)(( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4));
        char_array_3[1] = (char)(((char_array_4[1] & 0xf ) << 4) + ((char_array_4[2] & 0x3c) >> 2));
        char_array_3[2] = (char)(((char_array_4[2] & 0x3 ) << 6) +   char_array_4[3]        );

        for (j = 0; j < (i - 1); j++)
            ret.push_back(char_array_3[j]);
    }

    return ret;
}
