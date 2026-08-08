#include "../server/model/offlinemodel.h"
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
        std::cout<<"mysql init failed\n";
        return -1;
    }


  
    OfflineMessageModel model;


    // 创建离线消息
    OfflineMessage message;


    // 模拟：
    // 用户1发送了一条消息
    // 用户2当前离线
    // 保存给用户2

    message.setUserId(2);

    // 假设对应 message 表里面 id=1 的消息
    message.setMessageId(1);


    message.setCreateTime(
        std::chrono::system_clock::now()
    );



    // 1. 插入

    if(model.insert(message))
    {
        std::cout
            << "insert offline message success"
            << std::endl;
    }
    else
    {
        std::cout
            << "insert offline message failed"
            << std::endl;

        return 0;
    }



    // 2. 查询用户2的离线消息

    auto messages =
        model.findByUserId(2);



    std::cout
        << "====== offline messages ======"
        << std::endl;


    for(auto& msg:messages)
    {

        std::cout
            << "id="
            << msg.id()

            << " user="
            << msg.userId()

            << " messageId="
            << msg.messageId()

            << std::endl;
    }



    if(messages.empty())
    {
        std::cout
            << "query failed"
            << std::endl;

        return 0;
    }



    // 3. 删除第一条

    auto id =
        messages[0].id();



    if(model.remove(id))
    {
        std::cout
            << "remove success"
            << std::endl;
    }
    else
    {
        std::cout
            << "remove failed"
            << std::endl;
    }



    // 4. 再查询

    auto after =
        model.findByUserId(2);



    std::cout
        << "after remove size="
        << after.size()
        << std::endl;



    return 0;
}