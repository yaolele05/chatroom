#pragma once
#include "../../../common/protocol/message.h"

class Session;
class HistoryService
{
    public:
    static HistoryService& instance();
    static void registerHandler();
    void historyRequest(const Message& msg,Session* se);

    private:
    HistoryService()=default;
};