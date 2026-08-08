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


    Message msg1;

    msg1.setSendId(1);
    msg1.setReceiverId(0);
    msg1.setGroupId(100);

    msg1.setType(MessageType::Text);
    msg1.setContent("hello group from user1");

    msg1.setSendTime(
        std::chrono::system_clock::now()
    );


    model.insert(msg1);



    Message msg2;

    msg2.setSendId(2);
    msg2.setReceiverId(0);
    msg2.setGroupId(100);

    msg2.setType(MessageType::Text);
    msg2.setContent("hello group from user2");

    msg2.setSendTime(
        std::chrono::system_clock::now()
    );


    model.insert(msg2);



    std::cout
    <<"======== group history ========\n";


    auto messages =
        model.findGroupHistory(
            100,
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
        <<" group="
        <<msg.groupId()
        <<" content="
        <<msg.content()
        <<std::endl;
    }


    return 0;
}