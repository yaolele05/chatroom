#pragma once
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <stdexcept>
class MysqlResult
{
public:

    explicit MysqlResult(MYSQL_STMT* stmt);
    ~MysqlResult();
    MysqlResult(const MysqlResult&) = delete;
    MysqlResult& operator=(const MysqlResult&) = delete;
    MysqlResult(MysqlResult&&) noexcept;
    MysqlResult& operator=(MysqlResult&&) noexcept;

    // 获取下一行
    bool fetch();
    // 行数
    size_t rowCount() const;
    // 列数
    size_t fieldCount() const;
    bool empty() const;
    bool valid() const;
    template<typename T>
    T get(int index) const;

    bool isNull(int index) const;


private:

    void bindResult();

    MYSQL_STMT* stmt_{nullptr};
    MYSQL_RES* metadata_{nullptr};
    std::vector<MYSQL_BIND> binds_;
    std::vector<unsigned long> lengths_;
    std::vector<char> nullFlags_;
    std::vector<enum_field_types> types_;
    std::vector<std::vector<char>> buffers_;
    static constexpr std::size_t kMaxBuffer = 4096;
};

template<>
inline int32_t MysqlResult::get<int32_t>(int index) const
{
    if(index < 0 ||index >= static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get<int32_t>");
    }

    if(isNull(index))
    {
        return 0;
    }

    switch(types_[index])
    {
    case MYSQL_TYPE_TINY:
        return static_cast<int32_t>(*reinterpret_cast<const int8_t*>(buffers_[index].data()));
    case MYSQL_TYPE_SHORT:
        return static_cast<int32_t>(*reinterpret_cast<const int16_t*>(buffers_[index].data()));

    case MYSQL_TYPE_LONG:
        return *reinterpret_cast<const int32_t*>(buffers_[index].data());

    case MYSQL_TYPE_LONGLONG:
        return static_cast<int32_t>(*reinterpret_cast<const int64_t*>(buffers_[index].data()));

    default:
        throw std::runtime_error("MysqlResult: invalid int32 type");
    }

    
}
template<>
inline int64_t MysqlResult::get<int64_t>(int index) const
{
    if(index < 0 ||
       index >= static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get<int64_t>");
    }

    if(isNull(index))
    {
        return 0;
    }

    switch(types_[index])
    {
    case MYSQL_TYPE_TINY:
        return static_cast<int64_t>(*reinterpret_cast<const int8_t*>(buffers_[index].data()));

    case MYSQL_TYPE_SHORT:
        return static_cast<int64_t>(*reinterpret_cast<const int16_t*>(buffers_[index].data()));

    case MYSQL_TYPE_LONG:
        return static_cast<int64_t>(*reinterpret_cast<const int32_t*>(buffers_[index].data() ));

    case MYSQL_TYPE_LONGLONG:
        return *reinterpret_cast<const int64_t*>(buffers_[index].data());

    default:
        throw std::runtime_error("MysqlResult: invalid int64 type");
    }
}
template<>
inline uint64_t MysqlResult::get<uint64_t>(int index) const
{
    if(index < 0 ||index >= static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get<uint64_t>");
    }

    if(isNull(index))
    {
        return 0;
    }

    switch(types_[index])
    {
    case MYSQL_TYPE_TINY:
        return static_cast<uint64_t>(*reinterpret_cast<const uint8_t*>(buffers_[index].data()));

    case MYSQL_TYPE_SHORT:
        return static_cast<uint64_t>(*reinterpret_cast<const uint16_t*>(buffers_[index].data()));

    case MYSQL_TYPE_LONG:
        return static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(buffers_[index].data()));

    case MYSQL_TYPE_LONGLONG:
        return *reinterpret_cast<const uint64_t*>(buffers_[index].data());

    default:
        throw std::runtime_error("MysqlResult: invalid uint64 type");
    }
}
template<>
inline float MysqlResult::get<float>(int index) const
{
   if(index < 0 ||index >= static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get<float>");
    }

    if(isNull(index))
    {
        return 0.0f;
    }

    if(types_[index] != MYSQL_TYPE_FLOAT)
    {
        throw std::runtime_error("MysqlResult: invalid float type");
    }

    return *reinterpret_cast<const float*>(buffers_[index].data());

}
template<>
inline double MysqlResult::get<double>(int index) const
{
     if(index < 0 ||index >= static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get<double>");
    }

    if(isNull(index))
    {
        return 0.0;
    }

    if(types_[index] != MYSQL_TYPE_DOUBLE)
    {
        throw std::runtime_error("MysqlResult: invalid double type");
    }

    return *reinterpret_cast<const double*>(buffers_[index].data()
    );

}
template<>
inline bool MysqlResult::get<bool>(int index) const
{
      if(index < 0 ||
       index >= static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get<bool>");
    }

    if(isNull(index))
    {
        return false;
    }

    switch(types_[index])
    {
    case MYSQL_TYPE_TINY:
        return *reinterpret_cast<const int8_t*>(buffers_[index].data()) != 0;

    case MYSQL_TYPE_SHORT:
        return *reinterpret_cast<const int16_t*>(buffers_[index].data()) != 0;

    case MYSQL_TYPE_LONG:
        return *reinterpret_cast<const int32_t*>(buffers_[index].data()) != 0;

    case MYSQL_TYPE_LONGLONG:
        return *reinterpret_cast<const int64_t*>(buffers_[index].data()) != 0;

    default:
        throw std::runtime_error("MysqlResult: invalid bool type");
    }

}
template<>
inline std::string MysqlResult::get<std::string>(int index) const
{
 if(index<0||index>=static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get<std::string>");
    }
    if(isNull(index))
    {
        return "";
    }
    return std::string(buffers_[index].data(),lengths_[index]);
}
