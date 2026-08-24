#include "session.h"
#include "../../minimuduo/net/Tcpconnection.h"
#include "../../common/protocol/Jsoncodec.h"
#include "../../common/protocol/packetcodec.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
Session::Session(const TcpConnectionptr& conn):conn_(conn)
{
 
}

void Session::send(const std::string& data)
{
    if(conn_)
    {
        conn_->send(data);
    }
}

void Session::send(const Message& message)
{
    send(PacketCodec::encode(JsonCodec::encode(message)));
}
void Session::send(const Message& message,const void* body,size_t len)
{
    send(PacketCodec::encode(JsonCodec::encode(message),body,len));
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