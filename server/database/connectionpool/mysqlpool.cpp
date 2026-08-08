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
    }

     cv_.notify_all();
}
bool MysqlPool::init(const std::string& host,uint16_t port,const std::string& user,const std::string& password,const std::string& database,size_t size)
{
     running_=true;
     std::cout<<"mysql pool init start"<<std::endl;
 for(int i=0;i<size;i++)
 {
    auto client=std::make_shared<MysqlClient>();
    if(!client->connect(host,port,user,password,database))
    {
        std::cerr<<"mysql connect failed"<<std::endl;
        return false;
    }
    std::cout<<"create mysql connection"<<i<<std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.push(client);
 }
   maxSize_=size;
  std::cout<<"mysql pool size="<<connections_.size()<<std::endl;

   return true;
}
std::shared_ptr<MysqlClient> MysqlPool::getConnection()
{
   std::unique_lock<std::mutex> lock(mutex_);
   std::cout<<"pool size="
             <<connections_.size()
             <<" running="
             <<running_
             <<std::endl;
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
  
      if (!conn->ping())
    {
        return;
    }

   {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!running_)
    return;
    connections_.push(conn);
   }

   cv_.notify_one();
}