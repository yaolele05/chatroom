#include "historyservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../session/usersession.h"
#include "../../model/messagemodel.h"
#include "../../model/usermodel.h"
#include <chrono>
#include "../../model/groupmodel.h"
void HistoryService::registerHandler()
{
    auto& dispatcher=BusinessDispatcher::instance();
    dispatcher.registerHandler(Messagetype::HistoryRequest,[](const Message& msg,Session* se)
{
  HistoryService::instance().historyRequest(msg,se);
});

}
HistoryService& HistoryService::instance()
{
    static HistoryService service;
    return service;
}
void HistoryService::historyRequest(const Message& msg, Session* se)
{
    if(!se)
        return;

    auto user = dynamic_cast<UserSession*>(se);
    if(!user)
        return;

    auto& payload = msg.payload();
    int type = payload["type"];
    Message response;
    response.setType(Messagetype::HistoryResponse);
    response.setSequence(msg.sequence());
    MessageModel model;
    nlohmann::json array = nlohmann::json::array();

    if(type == 1)
    {
      
        if(payload.value("blocked", false))
        {
            response.payload()["code"] = 1;
            response.payload()["message"] ="双方存在屏蔽关系，无法查看历史消息";
            se->send(response);
            return;
        }
        response.payload()["type"] = 1;
        int peerId = payload["peerId"];
        auto list =model.findPriHistory( user->userid(),peerId,50,0);

        for(auto& it : list)
        {
            nlohmann::json m;

            m["senderId"] = it.sendId();
            m["receiverId"] = it.receiverId();
            m["content"] = it.content();
            m["time"] =std::chrono::duration_cast<std::chrono::seconds>(it.sendTime().time_since_epoch()).count();

            array.push_back(m);
        }
    }

    else if(type == 2)
    {
        response.payload()["type"] = 2;
        int groupId = payload["groupId"];
        GroupModel groupModel;

        if(!groupModel.isGroupMember(groupId, user->userid()))
        {
            response.payload()["code"] = 1;
            response.payload()["message"] = "你不是该群成员，无法查看群聊历史";

            se->send(response);
            return;
        }

        auto list =  model.findGroupHistory( groupId,50, 0);

        UserModel userModel;

        for(auto& it : list)
        {
            nlohmann::json m;

            m["senderId"] = it.sendId();
            m["content"] = it.content();
            auto sender =userModel.findById(it.sendId());
            if(sender)
            {
                m["senderName"] = sender->username();
            }
            else
            {
                m["senderName"] ="用户" + std::to_string(it.sendId());
            }
            m["time"] = std::chrono::duration_cast<std::chrono::seconds>(it.sendTime().time_since_epoch()).count();

            array.push_back(m);
        }
    }
    else
    {
        response.payload()["code"] = 1;
        response.payload()["message"] ="未知的历史消息类型";
        se->send(response);
        return;
    }

    response.payload()["code"] = 0;
    response.payload()["type"] = type;
    response.payload()["message"] = array;
    se->send(response);
}