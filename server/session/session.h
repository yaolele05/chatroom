#pragma once
#include <string>
#include "../../minimuduo/net/callback.h"
#include <nlohmann/json.hpp>
#include "../../common/protocol/message.h"
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

   private:
   TcpConnectionptr conn_;

};