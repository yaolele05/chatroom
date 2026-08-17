#pragma  once
#include "../../../common/protocol/message.h"
#include "../../session/session.h"
class FriendService
{
    public:
    static FriendService& instance();
    static void registerHandler();
    void addFriend(const Message& msg,Session*s);
    void deleteFriend(const Message& msg,Session* s);
    void FriendList(const Message& msg,Session* s);
    bool blockFriend(const Message& msg,Session* s);
    bool unblockFriend(const Message& msg,Session* s);
    bool isFriendBlocked(const Message& msg,Session* s);

};