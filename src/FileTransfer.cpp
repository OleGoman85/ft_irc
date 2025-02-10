#include "../include/FileTransfer.hpp"

FileTransfer::FileTransfer()
    : _senderFd(-1),
      _receiverFd(-1),
      _filename(""),
      _filesize(0),
      _receivedBytes(0)
{
}

FileTransfer::FileTransfer(int senderFd, int receiverFd,
                           const std::string& filename, size_t filesize)
    : _senderFd(senderFd),
      _receiverFd(receiverFd),
      _filename(filename),
      _filesize(filesize),
      _receivedBytes(0)
{
}

int FileTransfer::getSenderFd() const
{
    return _senderFd;
}

int FileTransfer::getReceiverFd() const
{
    return _receiverFd;
}

const std::string& FileTransfer::getFilename() const
{
    return _filename;
}

size_t FileTransfer::getFilesize() const
{
    return _filesize;
}

size_t FileTransfer::getReceivedBytes() const
{
    return _receivedBytes;
}

void FileTransfer::appendData(const std::vector<char>& dataChunk)
{
    _fileBuffer.insert(_fileBuffer.end(), dataChunk.begin(), dataChunk.end());
    _receivedBytes += dataChunk.size();
}

bool FileTransfer::isComplete() const
{
    return _receivedBytes >= _filesize;
}

const std::vector<char>& FileTransfer::getFileBuffer() const
{
    return _fileBuffer;
}
