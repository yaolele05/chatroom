#include <iostream>

#include "../server/model/messagemodel.h"
#include "../server/database/connectionpool/mysqlpool.h"

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


    MessageModel model;


    // 插入测试消息1
    Message msg1;

    msg1.setSendId(1);
    msg1.setReceiverId(2);
    msg1.setGroupId(0);
    msg1.setType(MessageType::Text);
    msg1.setContent("hello tom");

    msg1.setSendTime(
        std::chrono::system_clock::now()
    );


    if(model.insert(msg1))
    {
        std::cout
        <<"insert msg1 success id="
        <<msg1.id()
        <<std::endl;
    }
    else
    {
        std::cout
        <<"insert msg1 failed\n";
    }



    // 插入测试消息2
    Message msg2;

    msg2.setSendId(2);
    msg2.setReceiverId(1);
    msg2.setGroupId(0);
    msg2.setType(MessageType::Text);
    msg2.setContent("hello jack");


    msg2.setSendTime(
        std::chrono::system_clock::now()
    );


    if(model.insert(msg2))
    {
        std::cout
        <<"insert msg2 success id="
        <<msg2.id()
        <<std::endl;
    }
    else
    {
        std::cout
        <<"insert msg2 failed\n";
    }



    std::cout
    <<"========= history =========\n";


    auto messages =
        model.findPriHistory(
            1,
            2,
            20,
            0
        );


    for(auto& msg:messages)
    {
        std::cout
        <<"id="
        <<msg.id()
        <<" sender="
        <<msg.sendId()
        <<" receiver="
        <<msg.receiverId()
        <<" content="
        <<msg.content()
        <<std::endl;
    }


    return 0;
}