#include "historyservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../session/usersession.h"
#include "../../model/messagemodel.h"
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
void HistoryService::historyRequest(const Message& msg,Session* se)
{
    if(!se)
    return;
    auto user=dynamic_cast<UserSession*>(se);
    if(!user)
    return;

    auto& payload=msg.payload();
    int type=payload["type"];

    Message response;
    response.setType(Messagetype::HistoryResponse);
    response.setSequence(msg.sequence());

    MessageModel model;
   nlohmann::json array=nlohmann::json::array();
      if(type==1)
      {

        int peerId=payload["peerId"];
        auto list=model.findPriHistory(user->userid(),peerId,50,0);
        for(auto& it:list)
        {
            nlohmann::json m;
            m["senderId"]=it.sendId();
            m["receiverId"]=it.receiverId();
            m["content"]=it.content();
            m["time"]=std::chrono::duration_cast<std::chrono::seconds>(it.sendTime().time_since_epoch()).count();
            array.push_back(m);
        }

      }
      else if(type==2)
      {
        int groupId=payload["groupId"];
        auto list=model.findGroupHistory(groupId,50,0);
        for(auto& it:list)
        {
            nlohmann::json m;
            m["senderId"]=it.sendId();
            m["content"]=it.content();
            array.push_back(m);
        }

      }
      response.payload()["message"]=array;
      se->send(response);


}