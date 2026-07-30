#pragma once
#include <string>
#include <chrono>

class User
{
    public:
     User()=default;
     int id() const;
     void setId(int id);
     const std::string& username() const;
     void setUsername(const std::string& username);
     const std::string& passwordHash() const;
     void setPasswordHash(const std::string& passwordHash);
     const std::string& nickname() const;
     void setNickname(const std::string& nickname);
     const std::string& avatar() const;
     void setAvatar(const std::string& avatar);
     const std::string& signature() const;
     void setSignature(const std::string& sign);
      bool online() const;
      void setOnline(bool online);

      std::chrono::system_clock::time_point createTime() const;
      void setCreateTime(const std::chrono::system_clock::time_point& time);

      std::chrono::system_clock::time_point updateTime() const;
      void setUpdateTime(const std::chrono::system_clock::time_point& time);


    private:
    int id_{0};
    std::string username_;
    std::string passwordHash_;
    std::string nickname_;
    std::string avatar_;
    std::string signature;
    bool online_{false};
    std::string createTime_;
};