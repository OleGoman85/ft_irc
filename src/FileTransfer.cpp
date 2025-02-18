#include "../include/FileTransfer.hpp"

/**
 * @brief Default constructor for FileTransfer.
 *
 * Initializes a file transfer session with default values.
 * The sender and receiver file descriptors are set to -1 (invalid),
 * the filename is empty, and the file size is set to 0.
 */
FileTransfer::FileTransfer()
    : _senderFd(-1),      ///< Initializes sender file descriptor as invalid (-1).
      _receiverFd(-1),    ///< Initializes receiver file descriptor as invalid (-1).
      _filename(""),      ///< Initializes an empty filename.
      _filesize(0),       ///< Sets file size to 0 (no file assigned yet).
      _receivedBytes(0)   ///< Initializes received byte count to 0.
{
}

/**
 * @brief Parameterized constructor for FileTransfer.
 *
 * Initializes a file transfer session with the given sender and receiver
 * file descriptors, filename, and expected file size.
 *
 * @param senderFd The file descriptor of the sender.
 * @param receiverFd The file descriptor of the receiver.
 * @param filename The name of the file being transferred.
 * @param filesize The total size of the file in bytes.
 */
FileTransfer::FileTransfer(int senderFd, int receiverFd,
                           const std::string& filename, size_t filesize)
    : _senderFd(senderFd),   ///< Assigns the sender file descriptor.
      _receiverFd(receiverFd), ///< Assigns the receiver file descriptor.
      _filename(filename),   ///< Stores the filename.
      _filesize(filesize),   ///< Stores the expected file size.
      _receivedBytes(0)      ///< Initializes received byte count to 0.
{
}

/**
 * @brief Retrieves the sender's file descriptor.
 *
 * @return The file descriptor of the sender.
 */
int FileTransfer::getSenderFd() const
{
    return _senderFd;
}

/**
 * @brief Retrieves the receiver's file descriptor.
 *
 * @return The file descriptor of the receiver.
 */
int FileTransfer::getReceiverFd() const
{
    return _receiverFd;
}

/**
 * @brief Retrieves the name of the file being transferred.
 *
 * @return A reference to the filename string.
 */
const std::string& FileTransfer::getFilename() const
{
    return _filename;
}

/**
 * @brief Retrieves the total size of the file being transferred.
 *
 * @return The file size in bytes.
 */
size_t FileTransfer::getFilesize() const
{
    return _filesize;
}

/**
 * @brief Retrieves the number of bytes received so far.
 *
 * @return The total number of bytes received.
 */
size_t FileTransfer::getReceivedBytes() const
{
    return _receivedBytes;
}

/**
 * @brief Appends a chunk of received data to the file buffer.
 *
 * This function adds the given data chunk to the internal file buffer
 * and updates the count of received bytes.
 *
 * @param dataChunk A vector containing the received data chunk.
 */
void FileTransfer::appendData(const std::vector<char>& dataChunk)
{
    _fileBuffer.insert(_fileBuffer.end(), dataChunk.begin(), dataChunk.end());
    _receivedBytes += dataChunk.size();
}

/**
 * @brief Checks if the file transfer is complete.
 *
 * A file transfer is considered complete when the number of received bytes
 * is equal to or greater than the total file size.
 *
 * @return true if the transfer is complete, false otherwise.
 */
bool FileTransfer::isComplete() const
{
    return _receivedBytes >= _filesize;
}

/**
 * @brief Retrieves the buffer storing received file data.
 *
 * @return A reference to the vector containing the received file data.
 */
const std::vector<char>& FileTransfer::getFileBuffer() const
{
    return _fileBuffer;
}

