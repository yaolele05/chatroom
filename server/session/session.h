#pragma once
#include <string>
#include "../../minimuduo/net/callback.h"
class Session
{

    public:
   Session (const TcpConnectionptr& conn );
    virtual ~Session() = default;

   void send(const std::string& msg);
   void close();
   bool connected() const;
   const TcpConnectionptr& connection() const;///

   private:
   TcpConnectionptr conn_;

};