#ifndef FILETRANSFER_HPP
#define FILETRANSFER_HPP

#include <string>
#include <vector>

/**
 * @brief Хранит информацию о передаче файла.
 *
 * Содержит необходимые поля для отслеживания, кто посылает файл, кто получает,
 * каков размер и какой прогресс отправки/получения.
 */
class FileTransfer {
public:
    FileTransfer();
    FileTransfer(int senderFd, int receiverFd,
                 const std::string& filename,
                 size_t filesize);

    // Сеттеры и геттеры
    int getSenderFd() const;
    int getReceiverFd() const;
    const std::string& getFilename() const;
    size_t getFilesize() const;
    size_t getReceivedBytes() const;

    // Добавляем кусок (чанк) сырых байт файла
    void appendData(const std::vector<char>& dataChunk);

    // Возвращает true, если объём уже принятых данных >= общего размера
    bool isComplete() const;

    // Возвращает весь «накопленный» файл
    const std::vector<char>& getFileBuffer() const;

private:
    int _senderFd;               ///< Кто отправляет (fd)
    int _receiverFd;             ///< Кто получает (fd)
    std::string _filename;       ///< Имя файла (как сообщил отправитель)
    size_t _filesize;            ///< Общий ожидаемый размер файла
    size_t _receivedBytes;       ///< Сколько уже получено сервером
    std::vector<char> _fileBuffer; ///< Накопленные данные файла
};

#endif // FILETRANSFER_HPP
