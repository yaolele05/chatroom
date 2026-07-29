#include "mysqlclient.h"
#include <iostream>
#include <memory>
class MysqlResult;

MysqlClient::MysqlClient()
{
    mysql_=mysql_init(nullptr);

    if(mysql_==nullptr)
    {
        std::cerr<<"mysql init failed"<<std::endl;
    }

}
MysqlClient::~MysqlClient()
{
    disconnect();
}
bool MysqlClient::connect(const std::string& host,uint16_t port,const std::string& username,const std::string& password, const std::string& database)
{
    if(mysql_==nullptr)
    return false;
    MYSQL*ret=mysql_real_connect(mysql_,host.c_str(),username.c_str(),password.c_str(),database.c_str(),port,nullptr,0);
    if(ret==nullptr)
    {
        std::cerr<<"mysql connect failed"<<std::endl;
        return false;
    }
     mysql_set_character_set(mysql_,"utf8mb4");
     return true;

}
void MysqlClient::disconnect()
{
  if(mysql_)
  {
    mysql_close(mysql_);
    mysql_=nullptr;
  }
}
bool MysqlClient::connected() const
{
    return mysql_ !=nullptr;
}
bool MysqlClient::execute(std::string_view sql)///
{
        if(mysql_==nullptr)
        return false;

        if(mysql_query(mysql_,sql.data())!=0)
        {
            std::cerr<<mysql_error(mysql_)<<std::endl;
            return false;
        }
   return true;
}
std::shared_ptr <MysqlResult> MysqlClient::query(std::string_view sql)
{
  if(mysql_==nullptr)
   return nullptr;

   if(mysql_query(mysql_,sql.data())!=0)
   {
    std::cerr<<mysql_error(mysql_)<<std::endl;
    return nullptr;
   }
    
   MYSQL_RES* res=mysql_store_result(mysql_);
   if(res==nullptr)
   return nullptr;

   return std::make_unique<MysqlResult>(res);
}
bool MysqlClient::beginTransaction()
{
    return execute("start transaction");
}
bool MysqlClient::commit()
{
    return execute("COMMIT");
}
bool MysqlClient::rollback()
{
    return execute("ROLLBACK");
}
std::string MysqlClient::error() const
{
    if(mysql_)
    {
        return mysql_error(mysql_);
    }
    return "mysql_ is null";
}