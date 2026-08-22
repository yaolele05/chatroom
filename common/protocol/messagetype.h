#pragma once
#include <cstdint>

enum class Messagetype : uint16_t
{
    unknown = 0,

    // 用户
    Register,
    RegisterResponse,

    Login,
    LoginResponse,

    Logout,
    LogoutResponse,

    // 好友
    AddFriend,
    AddFriendResponse,

    DeleteFriend,
    DeleteFriendResponse,

    FriendList,
    FriendListResponse,

    FriendOnlineStatus,

    // 私聊
    PrivateChat,
    PrivateChatResponse,

    // 群组
    CreateGroup,
    CreateGroupResponse,

    JoinGroup,
    JoinGroupResponse,

    LeaveGroup,
    LeaveGroupResponse,

    GroupChat,
    GroupChatResponse,

    GroupList,
    GroupListResponse,

    // 心跳
    HeartBeat,
    HeartBeatResponse,

    // 文件
    FileStart,
    GroupFileStart,
    FileChunk,
    FileFinish,

    FILE_ACK,
    FILE_RESUME_REQUEST,
    FILE_RESUME_RESPONSE,

    // 通用
    MessageAck,
    MessageRecall,
    MessageRead,

    Error,
    HistoryRequest,
    HistoryResponse,
    OfflineFileNotify,
    FileDownloadRequest,

    SendRegisterCode,
    SendRegisterCodeResponse,

    SendLoginCode,
    SendLoginCodeResponse,

    SendResetCode,
    SendResetCodeResponse,

   LoginByCode,
   LoginByCodeResponse,

   ResetPassword,
   ResetPasswordResponse,

   PrivateChatRead,
   PrivateUnreadRequest,

   BlockFriend,
   BlockFriendResponse,
   UnblockFriend,
   UnblockFriendResponse,

   AcceptFriend,
   AcceptFriendResponse,

   RejectFriend,
  RejectFriendResponse,

  FriendRequestList,
  FriendRequestListResponse,

  GroupJoinRequestList,
  GroupJoinRequestListResponse,
  AcceptGroupJoinRequest,
  AcceptGroupJoinRequestResponse,
  RejectGroupJoinRequest,
  RejectGroupJoinRequestResponse,
  SetGroupAdmin,
   SetGroupAdminResponse,

   GroupMemberList,
  GroupMemberListResponse,
    RemoveGroupMember,
    RemoveGroupMemberResponse,

    DeleteAccount,
    DeleteAccountResponse,

     FriendRequestNotify,
    GroupJoinRequestNotify,
   
    PrivateUnreadNotify,
    GroupUnreadNotify


};