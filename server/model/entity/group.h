#pragma once
#include<string>
#include <chrono>
class Group
{
    public:
    
    Group()=default;
    
    int id() const;
    void setId(int id);
    const std::string& name() const;
    void setName(const std::string& name);
    int ownerId() const;
    void setOwnerId(int ownerId);
    const std::string& notice() const;
    void setNotice(const std::string& notice);
    std::chrono::system_clock::time_point createTime() const;
    void setCreateTime(const std::chrono::system_clock::time_point& time);
    
    private:
    int groupId_{0};
    std::string groupName_;
    int ownerId_{0};
     std::string notice_;
     std::chrono::system_clock::time_point createTime_;
};