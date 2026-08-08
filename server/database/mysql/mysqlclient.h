#pragma once
#include <mysql/mysql.h>
#include  <string>
#include <string_view>
#include <memory>
#include "mysqlresult.h"
#include "mysqlstatement.h"
class MysqlClient
{
    public:
    MysqlClient();
    ~MysqlClient();

    MysqlClient(const MysqlClient&)=delete;
    MysqlClient& operator=(const MysqlClient&)=delete;

    MysqlClient(MysqlClient&&) noexcept;
    MysqlClient& operator=(MysqlClient&&) noexcept;///

    bool connect(const std::string& host,uint16_t port,const std::string& user,const std::string& password,const std::string& database);
    bool connected() const;
    void disconnect();
   
    bool execute(std::string_view sql);

    std::unique_ptr<MysqlStatement> prepare(std::string_view sql);
  
    bool beginTransaction();
    bool commit();
    bool rollback();
    int64_t lastInsertId() const;
    
    std::string error() const;
    bool ping();

    private:
    MYSQL* mysql_{nullptr};
    
      friend class MysqlStatement;

};