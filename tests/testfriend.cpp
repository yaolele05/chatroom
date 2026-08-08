#include <iostream>

#include "../server/model/friendmodel.h"
#include "../server/model/entity/friend.h"

#include "../server/database/connectionpool/mysqlpool.h"



int main()
{

    // 初始化mysql连接池

    if(!MysqlPool::instance().init(
        "127.0.0.1",
        3306,
        "chatserver",
        "123456",
        "chatroom",
        2))
    {
        std::cout
            <<"mysql init failed\n";

        return -1;
    }



    FriendModel model;



    /*
        测试数据:

        用户1
        用户2


        添加:

        1 -> 2

    */


    Friend relation;


    relation.setUserId(1);

    relation.setFriendId(2);

    relation.setStatus(1);

    relation.setCreateTime(
        std::chrono::system_clock::now()
    );



    if(model.addFriend(relation))
    {
        std::cout
            <<"add friend success\n";
    }
    else
    {
        std::cout
            <<"add friend failed\n";
    }



    /*
        测试 isFriend
    */


    bool ok =
        model.isFriend(1,2);


    std::cout
        <<"isFriend(1,2)="
        <<ok
        <<std::endl;



    /*
        测试 findFriends
    */


    auto friends =
        model.findFriends(1);


    std::cout
        <<"======= friends =======\n";


    for(auto& f:friends)
    {
        std::cout
            <<"id="
            <<f.id()
            <<" user="
            <<f.userId()
            <<" friend="
            <<f.friendId()
            <<" status="
            <<f.status()
            <<std::endl;
    }



    /*
        测试删除
    */


    if(model.removeFriend(1,2))
    {
        std::cout
            <<"remove success\n";
    }
    else
    {
        std::cout
            <<"remove failed\n";
    }



    /*
        删除后再次判断
    */


    bool after =
        model.isFriend(1,2);


    std::cout
        <<"after remove isFriend="
        <<after
        <<std::endl;



    return 0;
}