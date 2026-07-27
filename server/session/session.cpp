#include "session.h"
#include "../../minimuduo/net/Tcpconnection.h"
Session::Session(const TcpConnectionptr& conn):conn_(conn)
{
 
}
void  Session::send(const std::string& msg)
{
    if(conn_)
    {
        conn_->send(msg);
    }
}
void Session::close()
{
    if(conn_)
    {
        conn_->shutdown();
    }
}
bool Session::connected() const
{
    return conn_!=nullptr;
}
const TcpConnectionptr& Session:: connection() const
{
    return conn_;
}