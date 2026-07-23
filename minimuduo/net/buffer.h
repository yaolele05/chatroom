#pragma once
#include <vector>
#include <string>
class Buffer
{
    public:
    static const size_t kCheapPrepend=8;
    static const size_t kInitialSize=1024;
    explicit Buffer(size_t initialSize=kInitialSize);
    ~Buffer()=default;
    size_t readableBytes() const;
    size_t writeableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;

    char* beginWrite();
    void hasWritten(size_t len);
    void retrieve(size_t len);
    void retrieveAll();
    std::string retrieveAllAsString();
   
    void append(const char*data,size_t len);
    void append(const std::string& data);

    void prepend(const void*data,size_t len);

    ssize_t readFd(int fd,int*savedErrno);
    ssize_t writeFd(int fd,int*savedErrno); 

    private:

    char* begin();
    const char* begin() const;

    void ensureWriteablebytes(size_t len);
    void makeSpace(size_t len);

    std::vector<char>buffer_;
    size_t readIndex_;
    size_t writeIndex_;
};