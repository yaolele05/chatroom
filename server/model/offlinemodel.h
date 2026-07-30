#pragma once
#include <vector>
#include"entity/message.h"
class OfflineMessageModel
{
    public:
    bool insert(const Message& message);

    std::vector<Message> query(int userid);
    bool remove(int userid);
    bool clear(int userid);

};