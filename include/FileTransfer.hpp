#ifndef FILETRANSFER_HPP
#define FILETRANSFER_HPP
#include <string>
#include <vector>

/**
 * @brief Holds information about a file transfer session.
 *
 * This class tracks sender, receiver, filename, filesize, and any
 * accumulated file data.
 */
class FileTransfer {
public:
    /**
     * @brief Default constructor for an empty file transfer.
     */
    FileTransfer();

    /**
     * @brief Construct a file transfer object with all needed fields.
     *
     * @param senderFd   The file descriptor of the sender
     * @param receiverFd The file descriptor of the receiver
     * @param filename   The file name
     * @param filesize   The total expected size of the file (in bytes)
     */
    FileTransfer(int senderFd,
                 int receiverFd,
                 const std::string& filename,
                 size_t filesize);

    /**
     * @brief Returns the sender's file descriptor.
     */
    int getSenderFd() const;

    /**
     * @brief Returns the receiver's file descriptor.
     */
    int getReceiverFd() const;

    /**
     * @brief Returns the filename (as specified by the sender).
     */
    const std::string& getFilename() const;

    /**
     * @brief Returns the total expected size of the file (in bytes).
     */
    size_t getFilesize() const;

    /**
     * @brief Returns how many bytes we have currently received.
     */
    size_t getReceivedBytes() const;

    /**
     * @brief Appends a chunk of raw bytes into the file buffer.
     *
     * @param dataChunk A vector of bytes
     */
    void appendData(const std::vector<char>& dataChunk);

    /**
     * @brief Checks whether the received data meets or exceeds the total size.
     */
    bool isComplete() const;

    /**
     * @brief Provides read-only access to the accumulated file buffer.
     */
    const std::vector<char>& getFileBuffer() const;

private:
    int _senderFd;         ///< The sender's file descriptor
    int _receiverFd;       ///< The receiver's file descriptor
    std::string _filename; ///< The file name
    size_t _filesize;      ///< The declared file size
    size_t _receivedBytes; ///< How many bytes we've received so far
    std::vector<char> _fileBuffer; ///< The accumulated file data
};

#endif // FILETRANSFER_HPP
