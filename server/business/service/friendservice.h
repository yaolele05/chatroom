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
    bool isFriendBlocked(const Message& msg, Session* se);
    void acceptFriend(const Message& msg,Session* se);
    void rejectFriend(const Message& msg,Session* se);
     void friendRequestList(const Message& msg,Session* se);
};