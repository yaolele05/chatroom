#pragma once
#include <mysql/mysql.h>
class MysqlResult
{
    public:
    explicit MysqlResult(MYSQL_RES* res);
    ~MysqlResult();

    MysqlResult(const MysqlResult&)=delete;
    MysqlResult& operator=(const MysqlResult&)=delete;

    bool valid() const;
     bool empty() const;
    MYSQL_ROW fetchRow();
    size_t rowCount() const;
    size_t fieldCount() const;

    private:
    MYSQL_RES* result_{nullptr};
};