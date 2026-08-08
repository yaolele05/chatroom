#pragma once
#include <vector>
#include <string>
#include <optional>
#include "entity/user.h"
#include<mysql/mysql.h>
#include "../database/mysql/mysqlresult.h"
class UserModel
{
    public:
    bool insert( User& user);
    bool update(const User& user);
    bool remove(int userid);

    std::optional<User>findById(int userid);
    std::optional<User> findByName(const std::string& username);
    

    std::vector<User> findAll();

  private:

    User makeUser(const MysqlResult& result);
   

};