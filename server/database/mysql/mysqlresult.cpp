#include "mysqlresult.h"
#include "mysqlresult.h"


MysqlResult::MysqlResult(MYSQL_RES* res):result_(res)
{

}
MysqlResult::~MysqlResult()
{
    if(result_)
    {
        mysql_free_result(result_);

        result_=nullptr;
    }
}
MYSQL_ROW MysqlResult::fetchRow()
{
    if(result_==nullptr)
        return nullptr;

    return mysql_fetch_row(result_);
}
bool MysqlResult::valid() const
{
    return result_!=nullptr;
}
size_t MysqlResult::rowCount() const
{
    if(!result_)
    return 0;

    return mysql_num_rows(result_);
}
size_t MysqlResult::fieldCount() const
{
    if(!result_)
    return 0;
    return mysql_num_fields(result_);
}
bool MysqlResult::empty() const
{
    if(result_==nullptr)
        return true;
    return mysql_num_rows(result_)==0;
}