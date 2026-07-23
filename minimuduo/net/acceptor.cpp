#include "acceptor.h"
#include "eventloop.h"
#include "socket.h"
#include "channel.h"
#include "inetaddress.h"
#include <unistd.h>
#include <errno.h>
Acceptor::Acceptor(EventLoop*loop,const InetAddress& listenaddr):loop_(loop),acceptSocket_(Socket::createNonblockingSocket()),acceptChannel_(loop,acceptSocket_.fd()),listening_(false),newconnectCallback_(nullptr)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenaddr);
    acceptChannel_.ReadCallback(std::bind(&Acceptor::handleReading,this));
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}
void Acceptor::listen()
{
    if(!listening_)
    {
        listening_=true;
        acceptSocket_.listen();
        acceptChannel_.enableReading();
    }
}
void Acceptor::setNewconnectCallback(const NewconnectCallback& cb)
{
    newconnectCallback_=cb;

}
void Acceptor::handleReading()
{
   while(true)
   {
        InetAddress peeraddr;
        int connfd=acceptSocket_.accept(&peeraddr);
        if(connfd<0)
        {
            if(errno==EAGAIN||errno==EWOULDBLOCK)
            {
                break;
            }
            else
            {
                perror("accept error");
                break;
            }
        }
        if(newconnectCallback_)
        {
            newconnectCallback_(connfd,peeraddr);
        }
        else
        {
            ::close(connfd);
        }
   }
}