#include "buffer.h"
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <algorithm>
#include <cassert>
Buffer::Buffer(size_t initialSize):buffer_(kCheapPrepend+initialSize),readIndex_(kCheapPrepend),writeIndex_(kCheapPrepend)
{}

void Buffer::retrieve(size_t len)
{
    assert(len<=readableBytes());
    if(len<readableBytes())
    {
        readIndex_+=len;
    }
    else
    {
        retrieveAll();
    }
    
}
void Buffer::retrieveAll()
{
    readIndex_=kCheapPrepend;
    writeIndex_=kCheapPrepend;
}
std::string Buffer::retrieveAllAsString()
{
    std::string str(peek(),readableBytes());
    retrieveAll();
    return str;
}
const char* Buffer::peek() const
{
    return begin()+readIndex_;
}
char* Buffer::begin()
{
    
    return buffer_.data();
}
const char* Buffer::begin() const
{
    return buffer_.data();
}
size_t Buffer::readableBytes() const
{
    return writeIndex_-readIndex_;
}
size_t Buffer::writeableBytes() const
{
    return buffer_.size()-writeIndex_;
}
size_t Buffer::prependableBytes() const
{
    return readIndex_;
}
void Buffer::ensureWriteablebytes(size_t len)
{
    if(writeableBytes()<len)
    {
        makeSpace(len);
    }

}
void Buffer::makeSpace(size_t len)
{
    if(writeableBytes()+prependableBytes()<len+kCheapPrepend)
    {
        buffer_.resize(writeIndex_+len);
    }
    else
    {
        size_t readable=readableBytes();
        std::copy(begin()+readIndex_,begin()+writeIndex_,begin()+kCheapPrepend);
        readIndex_=kCheapPrepend;
        writeIndex_=readIndex_+readable;
    }
    
}
void Buffer::append(const char* data,size_t len)
{
    ensureWriteablebytes(len);
    std::copy(data,data+len,beginWrite());
    hasWritten(len);
}
void Buffer::append(const std::string& data)
{
    append(data.data(),data.length());
}
void Buffer::prepend(const void* data,size_t len)
{
    assert(len<=prependableBytes());
    readIndex_-=len;
    const char* d=static_cast<const char*>(data);
    std::copy(d,d+len,begin()+readIndex_);
}
char* Buffer::beginWrite()
{
       return begin()+writeIndex_;
}
void Buffer::hasWritten(size_t len)
{
    assert(len<=writeableBytes());
    writeIndex_+=len;
}
ssize_t Buffer::readFd(int fd,int* savedError)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable=writeableBytes();
    vec[0].iov_base=begin()+writeIndex_;
    vec[1].iov_base=extrabuf;
    vec[0].iov_len=writeable;
    vec[1].iov_len=sizeof(extrabuf);
    const int iovcnt=(writeable<sizeof(extrabuf))? 2: 1;
    const ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *savedError=errno;
    }
    else if(static_cast<size_t>(n)<=writeable)
    {
        hasWritten(n);
    }
    else
    {
        hasWritten(writeable);
        append(extrabuf,n-writeable);
    }

    return n;

}
ssize_t Buffer::writeFd(int fd,int* savedError)
{
    ssize_t n=::write(fd,peek(),readableBytes());
    if(n<0)
    {
        *savedError=errno;
    }
    else
    {
        retrieve(n);
    }
    return n;
}