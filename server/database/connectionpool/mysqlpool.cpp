#include "mysqlpool.h"
#include "../mysql/mysqlclient.h"

#include <iostream>
MysqlPool::MysqlPool()
{

}
MysqlPool& MysqlPool::instance()
{
 
    static MysqlPool pool;
    return pool;
}
MysqlPool::~MysqlPool()
{

    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_=false;

         while(! connections_.empty())
       {
        connections_.pop();
       }
    }

     cv_.notify_all();
}
bool MysqlPool::init(const std::string& host,uint16_t port,const std::string& user,const std::string& password,const std::string& database,size_t size)
{
     running_=true;
 for(int i=0;i<size;i++)
 {
    auto client=std::make_shared<MysqlClient>();
    if(!client->connect(host,port,user,password,database))
    {
        std::cerr<<"mysql connect failed"<<std::endl;
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.push(client);
 }
   maxSize_=size;
  

   return true;
}
std::shared_ptr<MysqlClient> MysqlPool::getConnection()
{
   std::unique_lock<std::mutex> lock(mutex_);
   cv_.wait(lock,[this](){
    return !connections_.empty() || !running_ ;
   });

     if(!running_)
     {
        return nullptr;
     }

     auto conn=connections_.front();
     connections_.pop();

     return conn;
}
void MysqlPool::releaseConnection(std::shared_ptr<MysqlClient> conn)
{
   if(! conn)
   return;

   {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!running_)
    return;
    connections_.push(conn);
   }

   cv_.notify_one();
}