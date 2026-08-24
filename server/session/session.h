#pragma once
#include <string>
#include "../../minimuduo/net/callback.h"
#include <nlohmann/json.hpp>
#include "../../common/protocol/message.h"
#include <atomic>
using json=nlohmann::json;
class Session
{

    public:
   Session (const TcpConnectionptr& conn );
    virtual ~Session() = default;

    virtual bool authenticated() const
   {
    return false;
   }
   virtual int userid() const
   {
    return -1;
   }
    void send(const std::string& data);
   void send(const Message& msg);
   void send(const Message& msg,const void*body,size_t len);
   void close();
   bool connected() const;
   const TcpConnectionptr& connection() const;///
   

    void markDisconnected();

   private:
   TcpConnectionptr conn_;
    
    std::atomic<bool>  connectionAlive_{true};//duoge线程访问

};