#include "mysqlresult.h"
#include<cstring>
#include <iostream>
#include <algorithm>
MysqlResult::MysqlResult(MYSQL_STMT* stmt):stmt_(stmt)
{

    if(!stmt_)
    return;

     if (mysql_stmt_store_result(stmt_) != 0)
   {
    throw std::runtime_error(mysql_stmt_error(stmt_));
   }
    metadata_=mysql_stmt_result_metadata(stmt_);

    if(!metadata_)
    return;

    bindResult();
}
MysqlResult::~MysqlResult()
{
    if(metadata_)
    {
        mysql_free_result(metadata_);
        metadata_=nullptr;
    }
}
bool MysqlResult::isStringType(enum_field_types type) const
{
    switch (type)
    {
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:

    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:

    case MYSQL_TYPE_JSON:

        return true;

    default:
        return false;
    }
}

void MysqlResult::bindResult()
{
   if(!metadata_)
   {
    return;
   }
   const unsigned int fieldCount=mysql_num_fields(metadata_);
   MYSQL_FIELD* fields=mysql_fetch_fields(metadata_); 
  binds_.assign(fieldCount, MYSQL_BIND{});
   lengths_.resize(fieldCount);
   nullFlags_.resize(fieldCount);
   types_.resize(fieldCount);
   buffers_.resize(fieldCount);
   longBuffers_.resize(fieldCount);
  
  for(unsigned int i=0;i<fieldCount;++i)
  {
    types_[i]=fields[i].type;
    MYSQL_BIND& bind=binds_[i];
  

  switch(fields[i].type)
  {
    case MYSQL_TYPE_TINY:
    buffers_[i].resize(sizeof(int8_t));
    break;

    case MYSQL_TYPE_SHORT:
    buffers_[i].resize(sizeof(int16_t));
    break;

    case MYSQL_TYPE_LONG:
    buffers_[i].resize(sizeof(int32_t));
    break;
  
    case MYSQL_TYPE_LONGLONG:
    buffers_[i].resize(sizeof(int64_t));
    break;

    case MYSQL_TYPE_FLOAT:
    buffers_[i].resize(sizeof(float));
    break;

    case MYSQL_TYPE_DOUBLE:
    buffers_[i].resize(sizeof(double));
    break;

    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIMESTAMP:
    buffers_[i].resize(sizeof(MYSQL_TIME));
    break;

    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_JSON:
    buffers_[i].resize(std::min<std::size_t>(fields[i].length+1,kMaxBuffer));
    break;

    default:
    buffers_[i].resize(std::min<std::size_t>(fields[i].length+1,kMaxBuffer));
    break;

  }
  std::memset(&bind,0,sizeof(MYSQL_BIND));
  bind.buffer_type = fields[i].type;
  bind.buffer = buffers_[i].data();
  bind.buffer_length =static_cast<unsigned long>(buffers_[i].size());
  bind.length = &lengths_[i];
  bind.is_null = reinterpret_cast<bool*>(&nullFlags_[i]);
  bind.is_unsigned = (fields[i].flags & UNSIGNED_FLAG) != 0;

 }
   if (mysql_stmt_bind_result(stmt_, binds_.data()) != 0)
   {
    throw std::runtime_error(mysql_stmt_error(stmt_));
    }
}

bool MysqlResult::fetchLongColumn(int index)
{
    if (index < 0 || index >= static_cast<int>(buffers_.size()))
    {
        return false;
    }

    if (nullFlags_[index])
    {
        return true;
    }

    const unsigned long actualLength = lengths_[index];

    if (actualLength <= buffers_[index].size())
    {
        return true;
    }

    std::cout << "[MysqlResult] long field detected"<< " index=" << index << " length=" << actualLength<< " oldBuffer=" << buffers_[index].size()<< std::endl;

    longBuffers_[index].resize(actualLength);

    MYSQL_BIND bind{};
    bind.buffer_type = types_[index];
    bind.buffer = longBuffers_[index].data();
    bind.buffer_length =static_cast<unsigned long>(longBuffers_[index].size());

    unsigned long length = 0;
    bool isNull = false;

    bind.length = &length;
    bind.is_null = reinterpret_cast<bool*>(&isNull);

    if (mysql_stmt_fetch_column(stmt_,&bind,static_cast<unsigned int>(index),0) != 0)
    {
        std::cerr << "mysql_stmt_fetch_column failed: "<< mysql_stmt_error(stmt_)<< std::endl;
        return false;
    }
   
    return true;
}
bool MysqlResult::fetch()
{
    if (!stmt_)
    {
        return false;
    }
    for (auto& buffer : longBuffers_)
    {
    buffer.clear();
    }
    int ret = mysql_stmt_fetch(stmt_);
    std::cout<<"mysql_stmt_fetch ret="<<ret <<std::endl;
    if (ret == 0)
    {
        for(size_t i=0;i<buffers_.size();++i)
    {
        std::cout<<"field " <<i<<" length="<<lengths_[i] <<" null="<<(int)nullFlags_[i]<<std::endl;
    }
        return true;
    }
    if (ret == MYSQL_NO_DATA)
    {
        return false;
    }
    if (ret == MYSQL_DATA_TRUNCATED)
   {
    std::cerr << "mysql_stmt_fetch: MYSQL_DATA_TRUNCATED" << std::endl;
    for (size_t i = 0; i < buffers_.size(); ++i)
    {
        if(nullFlags_[i])
        {
            continue;
        }
        if(lengths_[i]>buffers_[i].size())
        {
            if(!isStringType(types_[i]))
            {
               
          std::cerr<< "[MysqlResult] "<< "unexpected truncated "   << "non-string field "   << "index="<< i<< std::endl;
        throw std::runtime_error("MysqlResult: "  "unexpected data truncation");
            }
            if(!fetchLongColumn(static_cast<int>(i)))
            {
                throw std::runtime_error( "MysqlResult: " "fetch long column failed");
            }
        }
        
    }

    return true;
    }
    throw std::runtime_error(mysql_stmt_error(stmt_));
}
size_t MysqlResult::rowCount() const
{
  if (!stmt_)
    {
        return 0;
    }

    return mysql_stmt_num_rows(stmt_);
}
size_t MysqlResult::fieldCount() const
{
   if(!metadata_)
   return 0;
   return mysql_num_fields(metadata_);
}
bool MysqlResult::valid() const
{
    return stmt_ != nullptr && metadata_ != nullptr;
}
bool MysqlResult::empty() const
{
    return rowCount() == 0;
}
bool MysqlResult::isNull(int index) const
{
    if(index < 0 ||index >= static_cast<int>(nullFlags_.size()))
    {
        return true;
    }

    return nullFlags_[index];
}