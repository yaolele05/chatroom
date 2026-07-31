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
    if(index < 0 ||
       index >= static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get");
    }

    return *reinterpret_cast<const int32_t*>(buffers_[index].data());
}
template<>
inline int64_t MysqlResult::get<int64_t>(int index) const
{
    if(index<0||index>=static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get");
    }
    return *reinterpret_cast<const int32_t*>(buffers_[index].data());
}
template<>
inline float MysqlResult::get<float>(int index) const
{
     if(index<0||index>=static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get");
    }
    return *reinterpret_cast<const float*>(buffers_[index].data());

}
template<>
inline double MysqlResult::get<double>(int index) const
{
     if(index<0||index>=static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get");
    }
    return *reinterpret_cast<const double*>(buffers_[index].data());

}
template<>
inline bool MysqlResult::get<bool>(int index) const
{
     if(index<0||index>=static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get");
    }
    return *reinterpret_cast<const bool*>(buffers_[index].data());

}
template<>
inline std::string MysqlResult::get<std::string>(int index) const
{
 if(index<0||index>=static_cast<int>(buffers_.size()))
    {
        throw std::out_of_range("MysqlResult::get");
    }
    return std::string(buffers_[index].data(),lengths_[index]);
}
