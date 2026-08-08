#include "../server/database/connectionpool/mysqlpool.h"
#include "../server/model/usermodel.h"

#include <iostream>

int main()
{
    bool ok = MysqlPool::instance().init(
        "127.0.0.1",
        3306,
        "root",
        "123456",
        "chatroom",
        5);

    if (!ok)
    {
        std::cout << "pool init failed\n";
        return 0;
    }

    UserModel model;

    // 后面开始测试

      User user;

user.setUsername("tom");

user.setPasswordHash("123456");

user.setNickname("Tom");

user.setAvatar("avatar.png");

user.setSignature("hello");



if(model.insert(user))
{
    std::cout << "insert success\n";

    std::cout << "id = "
              << user.id()
              << std::endl;
}
else
{
    std::cout << "insert failed\n";
}
    return 0;
}