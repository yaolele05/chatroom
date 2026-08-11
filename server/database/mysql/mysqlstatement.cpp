#include "mysqlstatement.h"
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mysql/mysql.h>
MysqlStatement::MysqlStatement(MYSQL* mysql):stmt_(nullptr)
{
    stmt_=mysql_stmt_init(mysql);
    if(stmt_ ==nullptr)
    {
        throw std::runtime_error("mysql_stmt_init failed");
    }
}
MysqlStatement::~MysqlStatement()
{
    if(stmt_)
    {
        mysql_stmt_close(stmt_);
    }
}
bool MysqlStatement::prepare(const std::string_view sql)
{
    if(mysql_stmt_prepare(stmt_,sql.data(),sql.size())!=0)
    {
     std::cerr<<"prepared failed "<< mysql_stmt_error(stmt_) <<std::endl;
     return false;
    }
    auto count=mysql_stmt_param_count(stmt_);
    binds_.assign(count,MYSQL_BIND{});
    values_.resize(count);
    return true;

}
bool MysqlStatement::bind(int index,const std::string& value)
{
    if(index<0 ||index>=static_cast<int>(binds_.size()))
    return false;

    values_[index]=value;
    auto& str =std::get<std::string>(values_[index]);

    binds_[index].buffer_type=MYSQL_TYPE_STRING;

    binds_[index].buffer=const_cast<char*>(str.data());
    binds_[index].buffer_length=str.size();

    return true;

}
bool MysqlStatement::bind(int index,int32_t value)
{
    if(index<0 || index>=static_cast<int>(binds_.size()))
    {
        return false;
    }
    values_[index]=value;
    auto& va=std::get<int32_t>(values_[index]);
    MYSQL_BIND& bind = binds_[index];
     
    bind.buffer_type=MYSQL_TYPE_LONG;
    bind.buffer=&va;
     bind.buffer_length=sizeof(va);

     return true;
}
bool MysqlStatement::bind(int index,bool value)
{
    if(index<0 ||index>=static_cast<int>(binds_.size()))
    return false;

    values_[index]=value;

    auto &va=std::get<bool>(values_[index]);
    MYSQL_BIND& bind=binds_[index];
    bind.buffer_type=MYSQL_TYPE_TINY;
    bind.buffer= &va;
    bind.buffer_length =sizeof(va);

   return true;
}

bool MysqlStatement::bind(int index,double value)
{
    if(index<0 ||index>=static_cast<int>(binds_.size()))
    return false;

    values_[index]=value;
        auto &va=std::get<double>(values_[index]);
    MYSQL_BIND& bind=binds_[index];
     bind.buffer_type = MYSQL_TYPE_DOUBLE;
    bind.buffer= &va;
    bind.buffer_length =sizeof(va);
    
      return true;
}
bool MysqlStatement::bind(int index,int64_t value)
{
    if(index<0 ||index>=static_cast<int>(binds_.size()))
    return false;

    values_[index]=value;
        auto &va=std::get<int64_t>(values_[index]);
    MYSQL_BIND& bind=binds_[index];
     bind.buffer_type = MYSQL_TYPE_LONGLONG;
    bind.buffer= &va;
    bind.buffer_length =sizeof(va);
    
      return true;
}
bool MysqlStatement::bind(int index, uint64_t value)
{
    if(index<0 ||index>=static_cast<int>(binds_.size()))
    return false;

    values_[index]=value;
        auto &va=std::get<uint64_t>(values_[index]);
    MYSQL_BIND& bind=binds_[index];
     bind.buffer_type = MYSQL_TYPE_LONGLONG;
    bind.buffer= &va;
    bind.buffer_length =sizeof(va);
    
      return true;
}
bool MysqlStatement::execute()
{

    if(!binds_.empty())
    {
        if(mysql_stmt_bind_param(stmt_,binds_.data())!=0)
        {
            std::cerr<<"bind failed"<<mysql_stmt_error(stmt_)<<std::endl;
            return false;
        }
       
    }
    if(mysql_stmt_execute(stmt_)!=0)
    {
        std::cerr<<"execute failed:"<<mysql_stmt_error(stmt_)<<std::endl;
        return false;
    }

    return true;
}
MysqlResult MysqlStatement::query()
{
    if (!execute())
        throw std::runtime_error("query failed");

    return MysqlResult(stmt_);
}