#include "../server/model/groupmodel.h"

#include "../server/model/entity/group.h"
#include "../server/model/entity/groupmember.h"
#include "../server/database/connectionpool/mysqlpool.h"
#include <iostream>
#include <chrono>


int main()
{


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


    GroupModel model;


    /*
        1. 创建群
    */

    Group group;

    group.setName("C++交流群");

    group.setOwnerId(1);

    group.setAvatar("");

    group.setDescription("学习C++和Linux");

    group.setCreateTime(
        std::chrono::system_clock::now()
    );


    if(model.create(group))
    {
        std::cout
            << "create group success"
            << std::endl;

        std::cout
            << "group id="
            << group.id()
            << std::endl;
    }
    else
    {
        std::cout
            << "create group failed"
            << std::endl;

        return 0;
    }



    /*
        2. 查询群
    */

    auto result=model.findById(group.id());


    if(result)
    {
        std::cout
            << "find group success"
            << std::endl;

        std::cout
            << "name="
            << result->name()
            << std::endl;

        std::cout
            << "owner="
            << result->OwnerId()
            << std::endl;
    }



    /*
        3. 查看群成员

        这里应该已经有群主
    */

    auto members =
        model.findGroupMembers(group.id());


    std::cout
        << "====== members ======"
        << std::endl;


    for(auto& member:members)
    {
        std::cout
            << "user="
            << member.userId()
            << " role="
            << static_cast<int>(
                member.role()
            )
            << std::endl;
    }



    /*
        4. 添加普通成员
    */


    GroupMember member;


    member.setGroupId(
        group.id()
    );


    member.setUserId(2);


    member.setRole(
        GroupRole::Member
    );


    member.setCreateTime(
        std::chrono::system_clock::now()
    );



    if(model.addGroupMember(member))
    {
        std::cout
            << "add member success"
            << std::endl;


        std::cout
            << "member id="
            << member.id()
            << std::endl;
    }
    else
    {
        std::cout
            << "add member failed"
            << std::endl;
    }



    /*
        5. 再次查询成员
    */


    members =
        model.findGroupMembers(
            group.id()
        );


    std::cout
        << "====== after add ======"
        << std::endl;


    for(auto& member:members)
    {
        std::cout
            << "user="
            << member.userId()
            << " role="
            << static_cast<int>(
                member.role()
            )
            << std::endl;
    }



    /*
        6. 查询用户加入的群
    */


    auto groups =
        model.findUserGroups(2);


    std::cout
        << "====== user 2 groups ======"
        << std::endl;


    for(auto& g:groups)
    {
        std::cout
            << "group id="
            << g.id()
            << " name="
            << g.name()
            << std::endl;
    }



    /*
        7. 判断成员
    */


    bool ok =
        model.isGroupMember(
            group.id(),
            2
        );


    std::cout
        << "is member="
        << ok
        << std::endl;



    /*
        8. 删除成员
    */


    if(model.removeGroupMember(
        group.id(),
        2
    ))
    {
        std::cout
            << "remove member success"
            << std::endl;
    }


    return 0;
}