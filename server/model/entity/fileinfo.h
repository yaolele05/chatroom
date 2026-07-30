#pragma once
#include<cstdint>
#include <string>
#include <chrono>
class FileInfo
{
    public:
     FileInfo()=default;

     std::uint64_t id() const;
     void setId(std::uint64_t id);
     int ownerId() const;
     void setOwnerId(int ownerId);

     const std::string& filename() const;
     void setFilename(const std::string& filename);

     std::uint64_t filesize() const;
     void setFilesize(std::uint64_t size);

     const std::string& md5() const;
     void setMd5(const std::string& md5);

     const std::string& path() const;
     void setPath(const std::string& path);

     std::uint64_t offset() const;
     void setOffset(std::uint64_t offset);

     int status() const;
     void setStatus(int status);

     std::chrono::system_clock::time_point createTime() const;
     void setCreateTime(std::chrono::system_clock::time_point& time);

     private:
     std::uint64_t id_{0};
     int ownerId_{0};
     std::string filrname_;
     std::uint64_t filesize_{0};
     std::string md5;
     std::string path;
     std::uint64_t offset_{0};
     int status_{0};
     std::chrono::system_clock::time_point createTime_;


};