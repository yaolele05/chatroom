#pragma once
#include <string>
#include <chrono>

class Friend
{
    public:
     Friend()=default;
     int userId() const ;
     void setUserId(int userId);
     int friendId() const;
     void setFriendId(int friendId);
      
     const std::string& remark() const;
     void setRemark(const std::string& remark);

     std::chrono::system_clock::time_point createTime() const;
     void setCreateTime(const std::chrono::system_clock::time_point& time);

     
    private:
    int userId_;
    int friendId_;
    std::string remark;
    std::chrono::system_clock::time_point createTime_;

};