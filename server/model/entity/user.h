#pragma once
#include <string>
#include <chrono>

class User
{
public:

User() = default;

int id() const
    {
        return id_;
    }

    void setId(int id)
    {
        id_ = id;
    }

    const std::string& username() const
    {
        return username_;
    }

    void setUsername(const std::string& username)
    {
        username_ = username;
    }

    const std::string& passwordHash() const
    {
        return passwordHash_;
    }

    void setPasswordHash(const std::string& passwordHash)
    {
        passwordHash_ = passwordHash;
    }
   const std::string& email() const
   {
    return email_;
   }
   void setEmail(const std::string& email)
   {
       email_=email;
   }
    const std::string& nickname() const
    {
        return nickname_;
    }

    void setNickname(const std::string& nickname)
    {
        nickname_ = nickname;
    }

    const std::string& avatar() const
    {
        return avatar_;
    }

    void setAvatar(const std::string& avatar)
    {
        avatar_ = avatar;
    }

    const std::string& signature() const
    {
        return signature_;
    }

 void setSignature(const std::string& sign)
    {
        signature_ = sign;
    }
 std::chrono::system_clock::time_point createTime() const
    {
        return createTime_;
    }
void setCreateTime(const std::chrono::system_clock::time_point& time)
    {
        createTime_ = time;
    }

void setUpdateTime(const std::chrono::system_clock::time_point& time)
 {
        updateTime_ = time;
}

private:
    int id_{0};
    std::string username_;
    std::string passwordHash_;
    std::string email_;
   std::string nickname_;
  std::string avatar_;
    std::string signature_;
    std::chrono::system_clock::time_point createTime_;
    std::chrono::system_clock::time_point updateTime_;
};