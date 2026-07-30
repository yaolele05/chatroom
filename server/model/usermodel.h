#pragma once
#include <vector>
#include <string>
#include <optional>
#include "entity/user.h"
class UserModel
{
    public:
    bool insert(const User& user);
    bool update(const User& user);
    bool remove(int userid);

    std::optional<User>findById(int userid);
    std::optional<User> findByName(const std::string& username);
    bool exists(const std::string& username);
    bool updateOnlineStatus(int userid,bool online);

    std::vector<User> findAll();

};