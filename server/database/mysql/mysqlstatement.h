#pragma once
#include <mysql/mysql.h>
#include "mysqlresult.h"
#include <string>
#include <vector>
#include<cstdint>
#include <variant>
class MysqlStatement
{
    public:
    explicit MysqlStatement(MYSQL* mysql_);
   ~MysqlStatement();
      MysqlStatement(const MysqlStatement&) = delete;
    MysqlStatement& operator=(const MysqlStatement&) = delete;
    bool prepare (const std::string_view sql);
    bool bind(int index,const std::string& value );
    bool bind(int index,int value);
    bool bind(int index,bool value);
    bool bind(int index,double value);
    bool bind(int index,int64_t value);
    bool bind(int index, uint64_t value); 
    bool execute();
    uint64_t affectedRows() const;
  
    MysqlResult query();
    
   private:
    MYSQL_STMT* stmt_;
    std::vector<MYSQL_BIND> binds_;
    using BindValue =std::variant<std::monostate,std::string,int32_t,bool,double,int64_t,uint64_t>;

    std::vector<BindValue> values_;


};