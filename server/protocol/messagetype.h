#pragma once
#include <cstdint>

enum class MessageType:uint16_t 
{
   unknown=0,
    
   Register,Login,Logout,

   RegisterResponse,LoginResponse,

   AddFriend,DeleteFriend,FriendList, FriendOnlineStatus,

   PrivateChat,

   CreateGroup,JoinGroup,LeaveGroup,GroupChat,

   HeartBeat,

   FileStart,FileChunk,FileFinish,

   MessageAck, MessageRecall,MessageRead,

    Error,

  
};