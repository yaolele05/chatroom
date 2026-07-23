#include "socket.h"
#include "inetaddress.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "socket.h"
#include <unistd.h>

int Socket::createNonblockingSocket()
{
    int fd=::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,0);
    return fd;
}
Socket::Socket(int fd):fd_(fd)
{

}
Socket::~Socket()
{
    ::close(fd_);
}
int Socket::fd() const
{   
    return fd_;
}
void Socket::bindAddress(const InetAddress& addr)
{
    if(::bind(fd_,reinterpret_cast<const sockaddr*>(addr.getSockAddr()),sizeof(sockaddr_in)))
    {
        perror("bind error");
    }
}
void Socket::listen()
{
    if(::listen(fd_,128))
    {
        perror("listen error");
    }
}
int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    socklen_t len=sizeof(addr);
    int connfd=::accept4(fd_,reinterpret_cast<sockaddr*>(&addr),&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(connfd>=0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}
void Socket::setReuseAddr(bool on)
{
    int optval=on? 1:0;
    if(::setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval))<0)
    {
        perror("setsockopt SO_REUSEADDR");
    }
}
void Socket::setReusePort(bool on)
{
   int optval=on? 1:0;
   if(::setsockopt(fd_,SOL_SOCKET,SO_REUSEPORT,&optval,sizeof(optval))<0)
   {
    perror("setsockopt SO_REUSEPORT");
   }
}
void Socket::shutdownWrite()
{
    if(::shutdown(fd_,SHUT_WR)<0)
    {
        perror("shutdownWrite");
    }
}

ssize_t Socket::write(const void* buf, size_t len)
{
    return ::write(fd_, buf, len);
}