 #include "client.h"
#include <iostream>
#include "../minimuduo/net/eventloop.h"
#include <thread>
#include <limits>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
void loginMenu(Client& client);
void chatMenu(Client& client);
void friendMenu(Client& client);
void friendactionMenu(Client& client,int friendid,const std::string& friendname);
void groupMenu(Client& client);
void groupactionMenu(Client& client,int groupid,const std::string& groupname);
void receiveFriendFileMenu(Client& client,int friendid);
void receiveGroupFileMenu(Client& client,int groupid);
void receiveFileActionMenu(Client& client,int64_t fileId);

void historyMenu(Client& client);
void privateChatLoop(Client& client, int friendid, const std::string& friendname);
void groupChatLoop(Client& client, int groupid, const std::string& groupname);
void friendRequestMenu(Client& client);
void groupManageMenu(Client& client,int groupid, const std::string& groupname);
void groupJoinRequestMenu(Client& client,int64_t groupid);
void groupMemberMenu(Client& client,int64_t groupId);
void groupJoinRequestMenu(Client& client,int64_t groupId);
void groupMemberActionMenu(Client& client,int64_t groupId,int64_t userId,const std::string& username);
#include <iomanip>
#include <sstream>

std::string formatFileSize(uint64_t size)
{
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;
    std::ostringstream oss;
    double value = size;
    std::string unit = "B";
    if (size >= GB)
    {
        value = size / GB;
        unit = "GB";
    }
    else if (size >= MB)
    {
        value = size / MB;
        unit = "MB";
    }
    else if (size >= KB)
    {
        value = size / KB;
        unit = "KB";
    }
    if (unit == "B")
    {
        oss << size << " " << unit;
    }
    else
    {
        oss << std::fixed << std::setprecision(2)<< value << " " << unit;
    }
    return oss.str();
}

int main(int argc, char* argv[])
{

    EventLoop loop;
    Client client(&loop);
    std::string serverIp;
    uint16_t serverPort=8888;
    for(int i=1;i<argc;++i)
    {
        std::string arg=argv[i];
        if(arg=="--ip" && i+1<argc)
        {
            serverIp=argv[++i];
        }
        else if(arg=="--port" && i+1<argc)
        {
            serverPort=static_cast<uint16_t>(std::stoi(argv[++i]));
        }
    }
    std::cout<<"Connectiong to"<<serverIp<<":"<<serverPort<<std::endl;
    if(!client.connect(serverIp,serverPort))
    {
        std::cout<<"connection failed\n";
        return -1;
    }
    std:: thread menuThread([&client](){loginMenu(client);});
    std::cout<<"before loop"<<std::endl;
    loop.loop();
    std::cout<<"after loop"<<std::endl;
    menuThread.join();
   
    return 0;
}
void loginMenu(Client& client)
{
    while(true)
    {
    std::cout<<R"(

======== 聊天室 ========

1. 密码登录
2. 邮箱验证码登录
3. 注册
4. 找回密码

0. 退出

==========================

)";
      int op;
      if(!(std::cin>>op))
      {
        std::cin.clear();
        std::cin.ignore( std::numeric_limits<std::streamsize>::max(),'\n');
        std::cout<<"输入格式错误，请重试\n";
        continue;
      }
      switch(op)
      {
        case 1:
        {
            std::string username;
            std::string pwd;
            std::cout<<"请输入用户名：";
            std::cin>>username;///记得加限制字数
            std::cout<<"请输入密码：";
            std::cin>>pwd;
            
            client.loginByPassword(username,pwd);
            if(client.waitingLoginResult())
            {
                chatMenu(client);
            }
          break;

        }
        case 2:
        {
        std::string email;
        std::string code;
        std::cout<<"请输入邮箱：";
        std::cin>>email;
              std::cout<<"正在发送验证码......\n";
              client.sendLoginCode(email);
              if(!client.waitingLogincodeResult())
              {
                std::cout<<"验证码发送失败，请稍后再试。\n";
                break;
              }
              std::cout<<"验证码已经发送，请检查邮箱。\n";
              std::cout<<"请输入验证码：";
              std::cin>>code;
              client.loginByCode(email,code);
              if(client.waitingLoginResult())
              {
                chatMenu(client);

              }

              break;
        }
       case 3:
         {
                std::string username;
                std::string password;
                std::string email;
                std::string code;

                std::cout << "请输入用户名：";
                std::cin >> username;

                std::cout << "请输入密码：";
                std::cin >> password;

                std::cout << "请输入邮箱：";
                std::cin >> email;


        if(email.find('@') == std::string::npos || email.find('.') == std::string::npos)
         { 
         std::cout << "邮箱格式错误，请重新输入。" << std::endl;
         break;
          return;
         }
                std::cout << "正在发送验证码......\n";
                client.sendRegisterCode(email);

                if(!client.waitRegisterCodeResult())
                {
                    std::cout<< "验证码发送失败，请稍后再试。\n";
                    break;
                }

                std::cout<< "验证码已经发送，请检查邮箱。\n";
                std::cout << "请输入验证码：";
                std::cin >> code;

                client.registerUser(username,password,email, code);
                if(client.waitRegisterResult())
                {
                    std::cout
                        << "注册成功，请返回登录。\n";

                    break;
                }

                std::cout<< "注册失败，请检查用户名、邮箱或验证码。\n";

                break;
            } 
         case 4:
        {
             std::string email;
            std::string code;
            std::string newpassword;
            std::string conpassword;
            std::cout<<"========找回密码========\n";
            std::cout<<"请输入注册邮箱："<<std::endl;
            std::cin>>email;
            std::cout<<"正在发送验证码......\n";
            client.sendResetCode(email);
            if(!client.waitResetCodeResult())
            {
                  std::cout<<"验证码发送失败，请稍后再试。\n";
                break;
            }
            std::cout<<"验证码已经发送，请检查邮箱。\n";
              std::cout<<"请输入验证码：";
              std::cin>>code;
              std::cout<<"请输入新密码：";
              std::cin>>newpassword;
              std::cout<<"请再次输入新密码:";
              std::cin>>conpassword;
              if(newpassword!=conpassword)
              {
                 std::cout<<"两次输入的新密码不一样,请重试\n";
                 break;
              }
              if(newpassword.size()<6)
              {
                std::cout<<"密码不能小于6位，请重新输入\n";
                break;
              }
            client.resetPassword(email,code,newpassword);
            if(client.waitResetPasswordResult()==true)
            {
                std::cout<<"重置密码成功，请重新登录！\n";
            }
            
            break;
        }
        case 0:
        {
            client.disconnect();
            client.quit();
            return;
        }


       
      }

    }
}
void chatMenu(Client& client)
{
    while(client.isLogin())
    {
        std::cout<<R"(
======== 聊天室 ========

1. 好友
2. 添加好友
3. 删除好友
4.群
5.创建群
6.加群
7.退群
8.好友申请列表
9.注销

0.退出

==========================
        )";


        int op;
        std::cout<<"请选择：";
        if(!(std::cin>>op))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
             std::cout<<"输入错误，请重新输入\n";
        continue;
        }

        switch(op)
        {
            case 1:
            friendMenu(client);
            break;

            case 2:
            {
            uint32_t friendid=0;
              std::cout << "\n0. 返回\n";
          std::cout<<"请输入要加的好友的ID："<<std::flush;
    
            if(!(std::cin>>friendid))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                std::cout<<"输入的好友ID错误，请重新输入"<<std::flush;
                break;
            }
            if(friendid==0)
            {
              break;
            }
            client.addFriend(friendid);
            break;
           }

           case 3:
           {
            uint32_t friendid=0;
            std::cout << "\n0. 返回\n";
       std::cout<<"请输入要删除的好友的ID："<<std::flush;
            if(!(std::cin>>friendid))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                std::cout<<"输入错误，请重新输入"<<std::flush;
                break;
            }
              if(friendid==0)
            {
              break;
            }
            client.deleteFriend(friendid);
            break;
           }

           case 4:
           {
            groupMenu(client);
            break;
           }

           case 5:
           {
             std::string groupName;
             std::string description;
            std::cout<<"请输入要创建群的群名：";
             std::cin>>groupName;
            std::cout<<"请输入要创建群的群介绍：";
             std::cin>>description;
            client.createGroup(groupName, description);
            break;
           }

           case 6:
           {
            uint32_t gid;
            std::cout << "\n0. 返回\n";
            std::cout<<"输入要加入的groupid:"<<std::flush;
              if(!(std::cin>>gid))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                std::cout<<"输入的群ID错误，请重新输入"<<std::flush;
                break;
            }
            if(gid==0)
            {
                break;
            }
            client.joinGroup(gid);
            break;
           }

           case 7:
           {
               uint32_t gid;
               std::cout << "\n0. 返回\n";
            std::cout<<"输入要退出的groupid:"<<std::flush;
              if(!(std::cin>>gid))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                std::cout<<"输入的群ID错误，请重新输入"<<std::flush;
                break;
            }
            if(gid==0)
            {
                break;
            }
            client.leaveGroup(gid);
            break;
           }
          case 8:
          {
            friendRequestMenu(client);
            break;
          }
         
           case 9:
        {
        std::cout << "\n========== 注销账号 ==========\n";
        std::cout << "注销账号后，账号以及相关数据将被删除。\n";
        std::cout << "1. 确定注销\n";
        std::cout << "0. 返回\n";
        std::cout << "请选择：";
        int choice;
      if(!(std::cin >> choice))
       {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::cout << "输入错误\n";
        break;
      }
     if(choice == 0)
    {
        break;
    }
     if(choice != 1)
     {
        std::cout << "选择无效\n";
        break;
     }
    client.deleteAccount();
    if(client.waitDeleteAccountResult())
    {
        std::cout << "账号注销成功！\n";
        return;
    }
    std::cout << "账号注销失败，请稍后重试。\n";
    break;
    }
       
           case 0:
           {
            client.logout();
             return;
           }
           default:
           std::cout<<"选择无效\n";
           break;
            

        }
    }
}
void friendMenu(Client& client)
{
    while(client.tcpClient_->connected())
    {
  
    std::cout << "\n========== 好友 ==========\n";
       client.friendList();
       client.waitFriendList();

       std::cout <<"\n";
   
       uint32_t friendid;
   
   if(!(std::cin>>friendid))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::cout<<"输入错误，请重新输入:";
        continue;
    }
    if(friendid==0)
    {
        return;
    }
    
    const auto& friends=client.friends();
    std::string friendname;
    bool found=false;
    for(const auto& f:friends)
    {
       if(f.value("id",0)==friendid)
       {
        friendname=f.value("nickname","");
        found=true;
        break;
       }
    }
    if(!found)
    {
        std::cout<<"好友ID不存在\n";
        break;
    }
    friendactionMenu(client,friendid,friendname);
   }

}
void friendactionMenu(Client& client,int friendid,const std::string& friendname)
{
     while(client.tcpClient_->connected())
    {
     std::cout<<R"(
==========================
1. 聊天
2.查看历史聊天记录
3.发送文件
4.待接收文件
5.屏蔽好友
6.取消屏蔽
0.退出

==========================
        )";
        int op;
        std::cout<<"请选择："<<std::flush;
     if(!(std::cin>>op))
     {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::cout<<"输入错误，请重新输入"<<std::flush;
        break;
     }
        switch(op)
        {
            case 1:
          {
             privateChatLoop(client, friendid, friendname);
            break;
          }
          case 2:
          {
            client.privateHistory(friendid,friendname);
            break;
          }
          case 3:
          {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::string filename;
            std::cout<<"请输入文件路径："<<std::flush;
            std::cin>>filename;
            client.sendPrivateFile(friendid,filename);
            break;

          }
          case 4:
          {
            receiveFriendFileMenu(client,friendid);
            break;
          }
           case 5:
           client.blockFriend(friendid);
           break;
           case 6:
           client.unblockFriend(friendid);
           break;

          
          case 0:
          {
            return;

          }
          default:
          {
            std::cout<<"输入无效\n";
            break;
          }


        }
    }
    return;
}
 std::string readChatMessage()
{
    std::string message;
    std::getline(std::cin, message);
    return message;
}
void privateChatLoop(Client& client,int friendid,const std::string& friendname)
{
    client.enterPrivateChat(friendid,friendname);

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    std::cout << "\n========================================\n";
    std::cout << "与 " << friendname << " 聊天\n";
    std::cout << "输入 /quit 退出聊天\n";
    std::cout << "\n========================================\n";
   
    while(true)
    {
        
       std::string text = readChatMessage();
        if(text.empty())
        {
           continue;
        }
        if(text == "/quit")
        {
            client.leaveChat();
            std::cout << "已退出聊天\n";
            return;
        }
        client.privateChat(friendid, text);
    }
}
void groupMenu(Client& client)
{
    while(true)
    {
        client.groupList();
        uint32_t gid;
        
        if(!(std::cin>>gid))
        {
            std::cout<<"输入错误，请重新输入:"<<std::flush;
            continue;
        }
    if(gid==0)
    {
        return;
    }

    const auto& groups=client.groups();
    std::string groupname;
    bool found=false;
    for(const auto& g:groups)
    {
       if(g.value("groupId",0)==gid)
       {
        groupname=g.value("groupname","");
        found=true;
        break;
       }
    }
    if(!found)
    {
        std::cout<<"群ID不存在\n";
        return;
    }
        groupactionMenu(client,gid,groupname);
    }
    
    return;
}
void groupactionMenu(Client& client,int groupid,const std::string& groupname)
{

     while(true)
    {
     std::cout<<R"(

==========================
1. 聊天
2.查看历史聊天记录
3.发送文件
4.待接收文件
5.群管理
6.群成员
0.退出

==========================
        )";
        int op;
        std::cout<<"请选择："<<std::flush;
     if(!(std::cin>>op))
     {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::cout<<"输入错误，请重新输入"<<std::flush;
     }
        switch(op)
        {
            case 1:
          {
          groupChatLoop(client, groupid, groupname);
         break;
          
         }
          case 2:
          {
           
           client.groupHistory(groupid);
            break;
          }
          case 3:
          {
            std::string filename;
            std::cout<<"请输入文件路径："<<std::flush;
            std::cin>>filename;
            client.sendGroupFile(groupid,filename);
            break;
          }
          case 4:
          {
            receiveGroupFileMenu(client,groupid);
            break;
          }
            case 5:
            {
                groupManageMenu(client, groupid, groupname);
                break;
            }
            case 6:
            {
            groupMemberMenu(client,groupid);

            break;
            }
           case 0:
          
            return;
          default:
           {
            break;
           }
          
        }
    }
  

    return;
}
void groupChatLoop(Client& client, int groupid, const std::string& groupname)
{

    client.enterGroupChat(groupid);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "\n========================================\n";
    std::cout << "进入群聊：" << groupname << "\n";
    std::cout << "输入 /quit 退出群聊\n";
    std::cout << "\n========================================\n";
    while(true)
    {
        std::string text = readChatMessage();
        if(text.empty())
        {
            continue;
        }
        if(text == "/quit")
        {
            client.leaveChat();
            std::cout << "已退出群聊\n";
            return;
        }
        client.groupChat(groupid, text);
    }
}
void receiveFriendFileMenu(Client& client,int friendid)
{
    if(client.isFriendBlockedEitherWay(static_cast<uint32_t>(friendid)))
    {
        std::cout << "双方存在屏蔽关系，无法查看待接收文件\n";
        return;
    }
    while(true)
    {
        const auto& files = client.pendingReceiveFiles();
        std::cout << "\n========== 待接收文件 ==========\n";
        std::vector<int64_t> fileIds;
        int index = 1;
        
        for(const auto& file : files)
        {
            if(file.senderId != friendid)
                continue;
            if(file.groupId != 0)
               continue;
            if(file.accepted)
                continue;

            fileIds.push_back(static_cast<int64_t>(file.fileId));
            std::cout<< index<< ". "<< file.filename<< "       "<<formatFileSize(file.filesize)<< " \n";
            ++index;
        }
        
    if(fileIds.empty())
     {
      std::cout<< "当前没有待接收文件\n";
     }
        std::cout << "0. 返回\n";

        int choice;
        std::cout << "\n请选择："<<std::flush;
        if(!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cout << "输入错误,请重新输入\n";
            continue;
        }
       
        if(choice == 0)
            return;
        if(choice < 1 || choice > static_cast<int>(fileIds.size()))
        {
            std::cout<< "选择无效\n";
            continue;
        }
        int64_t fileId =fileIds[choice - 1];
        receiveFileActionMenu(client,fileId);
    }
}
void receiveGroupFileMenu(Client& client,int groupid)
{
    while(true)
    {
        const auto& files = client.pendingReceiveFiles();
        std::cout << "\n========== 待接收文件 ==========\n";
        std::vector<int64_t> fileIds;
        int index = 1;
        
        for(const auto& file : files)
        {
            if(file.groupId != groupid)
                continue;
                if(file.accepted)
                continue;

            fileIds.push_back(static_cast<int64_t>(file.fileId));
            std::cout<< index<< ". "<< file.filename<< "       "<< formatFileSize(file.filesize)<< " MB\n";
            ++index;
        }

        std::cout << "0. 返回\n";

        int choice;
        std::cout << "\n请选择："<<std::flush;
        if(!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cout << "输入错误,请重新输入\n";
            continue;
        }
       
        if(choice == 0)
            return;
        if(choice < 1 || choice > static_cast<int>(fileIds.size()))
        {
            std::cout<< "选择无效\n";
            continue;
        }
        int64_t fileId =fileIds[choice - 1];
        receiveFileActionMenu(client,fileId);
    }
}
void receiveFileActionMenu(Client& client,int64_t fileId)
{
    while(true)
    {
        std::cout << "\n========== 文件 ==========\n";
        std::cout << "1. 接收\n";
        std::cout << "2. 拒绝\n";
        std::cout << "0. 返回\n";

        int op;
        std::cout << "请选择：";
        if(!(std::cin >> op))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cout << "输入错误,请重新输入\n";
            continue;
        }
        switch(op)
        {
        case 1:
        {
            client.acceptFile(fileId);
            return;
        }
        case 2:
        {
             std::cout<< "已拒绝本次接收，"<< "文件仍保留在待接收列表\n";
            return;
        }
        case 0:
            return;

        default:
            std::cout << "选择无效\n";
            break;
        }
    }
}
void historyMenu(Client& client)
{
    while(client.tcpClient_->connected())
    {
        std::cout << "\n========== 历史消息 ==========\n";
        std::cout << "1. 查看历史消息\n";
        std::cout << "0. 返回\n";
        std::cout << "请选择：" << std::flush;

        int choice;
        if(!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            continue;
        }
        if(choice==0)
        {
            return;
        }
    }
}
std::string formatTimestamp(std::int64_t timestamp)
{
    std::time_t time = static_cast<std::time_t>(timestamp);

    std::tm* localTime = std::localtime(&time);

    if(localTime == nullptr)
    {
        return "未知时间";
    }
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}
void friendRequestMenu(Client& client)
{
    while(true)
    {
        std::cout << "\n========== 好友申请 ==========\n";

        client.friendRequestList();
        client.waitFriendRequestList();

        const auto& requests = client.friendRequests();

        if(requests.empty())
        {
            std::cout << "当前没有好友申请\n";
            std::cout << "0. 返回\n";

            int choice;
            std::cin >> choice;

            if(choice == 0)
            {
                return;
            }

            continue;
        }

        int index = 1;

        for(const auto& request : requests)
        {
            std::cout << "\n";
            std::cout << index << ".\n";

            std::cout<< "申请ID: "<< request.value("requestId", 0)<< "\n";
            std::cout<< "申请人ID: " << request.value("fromUserId", 0)<< "\n";
            std::cout<< "用户名: "<< request.value("username", "")<< "\n";
            std::cout<< "昵称: "<< request.value("nickname", "") << "\n";
            std::int64_t createTime =request.value("createTime", static_cast<std::int64_t>(0));
            std::cout<< "申请时间: "  << formatTimestamp(createTime)<< "\n";
            std::cout<< "-----------------------------\n";

            ++index;
        }

        std::cout << "0. 返回\n";
        std::cout << "请选择申请编号：" << std::flush;

        int choice;

        if(!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入错误，请重新输入\n";
            continue;
        }

        if(choice == 0)
        {
            return;
        }

        if(choice < 1 || choice > static_cast<int>(requests.size()))
        {
            std::cout << "选择无效\n";
            continue;
        }
        const auto& request = requests[choice - 1];
        uint32_t requestId =request.value("requestId", 0u);
        uint32_t userId =request.value("fromUserId", 0u);
        std::string username = request.value("nickname", request.value("username", ""));
        while(true)
        {
            std::cout << "\n";
            std::cout << "========== 好友申请 ==========\n";
            std::cout << "申请人: " << username << "\n";
            std::cout << "1. 同意\n";
            std::cout << "2. 拒绝\n";
            std::cout << "0. 返回\n";
            std::cout << "请选择：" << std::flush;

            int op;

         if(!(std::cin >> op))
          {
            std::cin.clear();
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(),  '\n' );
            std::cout << "输入错误，请重新输入\n";
            continue;
          }
         if(op == 0)
        {
            break;
        }
        if(op == 1)
        {
            client.acceptFriend(requestId);
                break;
        }
        if(op == 2)
            {
                client.rejectFriend(requestId);
                break;
            }
            std::cout << "选择无效\n";
        }
    }
}
void groupManageMenu(Client& client,int groupid,const std::string& groupname)
{
    while(true)
    {
        std::cout << "\n";
        std::cout << "========== 群管理 ==========\n";
        std::cout << "群名称：" << groupname << "\n";
        std::cout << "群ID：" << groupid << "\n";
        std::cout << "\n";

        std::cout << "1. 查看加群申请\n";
        std::cout << "2. 查看群成员\n";
      
        std::cout << "0. 返回\n";

        int op;

        std::cout << "请选择：" << std::flush;

        if(!(std::cin >> op))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cout << "输入错误，请重新输入\n";
            continue;
        }

        switch(op)
        {
            case 1:
            {
             groupJoinRequestMenu(client,groupid);
                break;
            }

            case 2:
            {
               groupMemberMenu(client,groupid);
                break;
            }
            case 0:
            {
                return;
            }

            default:
            {
                std::cout << "选择无效\n";
                break;
            }
        }
    }
}
void groupJoinRequestMenu(Client& client, int64_t groupid)
{
    while(true)
    {
        std::cout << "\n";
        std::cout << "========== 加群申请 ==========\n";

        client.groupJoinRequestList(groupid);
        client.waitGroupJoinRequestList();
        const auto& requests = client.groupJoinRequests();
        if(requests.empty())
        {
            std::cout << "当前没有待处理的加群申请\n";
            std::cout << "0. 返回\n";
            int choice;
        if(!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                continue;
        }
        if(choice == 0)
        {
            return;
        }
            continue;
        }

        int index = 1;

        for(const auto& request : requests)
        {
            std::cout << "\n";
            std::cout << index << ".\n";
            std::cout << "申请ID: "<< request.value("requestId", 0)<< "\n";
            std::cout<< "申请人ID: "<< request.value("fromUserId", 0)<< "\n";
            std::cout<< "用户名: "<< request.value("username", "")<< "\n";
            std::cout<< "昵称: "<< request.value("nickname", "")<< "\n";
            std::int64_t createTime =request.value("createTime", static_cast<std::int64_t>(0));
            std::cout<< "申请时间: "<< formatTimestamp(createTime)<< "\n";
            std::cout<< "-----------------------------\n";

            ++index;
        }

        std::cout << "0. 返回\n";
        std::cout << "请选择申请编号：" << std::flush;

        int choice;

        if(!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cout << "输入错误，请重新输入\n";
            continue;
        }

        if(choice == 0)
        {
            return;
        }

        if(choice < 1 ||
           choice > static_cast<int>(requests.size()))
        {
            std::cout << "选择无效\n";
            continue;
        }

        const auto& request = requests[choice - 1];
    int64_t requestId =request.value("requestId",static_cast<int64_t>(0));
        int userId =request.value("userId", 0);
        std::string username =request.value( "nickname",request.value("username", ""));
        while(true)
        {
            std::cout << "\n";
            std::cout << "========== 加群申请 ==========\n";
            std::cout << "申请人：" << username << "\n";
            std::cout << "用户ID：" << userId << "\n";
            std::cout << "1. 同意\n";
            std::cout << "2. 拒绝\n";
            std::cout << "0. 返回\n";

            int op;

            std::cout << "请选择：" << std::flush;

            if(!(std::cin >> op))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                std::cout << "输入错误，请重新输入\n";
                continue;
            }
            if(op == 0)
            {
                break;
            }
            if(op == 1)
            {
                client.acceptGroupJoinRequest(requestId);
                break;
            }

            if(op == 2)
            {
                client.rejectGroupJoinRequest(requestId);
                break;
            }

            std::cout << "选择无效\n";
        }
    }
}
void groupMemberMenu(Client& client,int64_t groupId)
{
    while(true)
    {
        std::cout << R"(

========== 群成员 ==========
1. 查看成员
0. 返回
============================

)";

        int op;
        std::cout << "请选择：" << std::flush;
        if(!(std::cin >> op))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入错误，请重新输入\n";
            continue;
        }

        if(op == 0)
            return;

        if(op == 1)
        {
            client.groupMemberList(groupId);
            client.waitGroupMemberList();

            const auto& members = client.groupMembers();
            std::cout << "\n========== 群成员 ==========\n";

            if(members.empty())
            {
                std::cout << "暂无成员\n";
                continue;
            }

            int index = 1;

        for(const auto& member : members)
        {
            int userId =member.value("userId",0);

            std::string username =member.value("nickname", member.value("username", ""));

            int role =member.value("role",0);
            std::cout<< index<< ". 【用户ID】: "<< userId<< "  "<<"【用户名】："<<username;

            if(role ==static_cast<int>(GroupRole::Owner))
            {
                    std::cout << "[群主]";
            }
            else if(role ==static_cast<int>(GroupRole::Admin))
            {
                    std::cout << "[管理员]";
             }
             else
            {
                    std::cout << "[成员]";
            }

                std::cout << "\n";
                ++index;
        }
        std::cout << "0. 返回\n";
        std::cout << "请选择成员：" << std::flush;

        int choice;
        if(!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cout << "输入错误，请重新输入\n";
            continue;
        }

        if(choice == 0)
            return;

        if(choice < 1 ||choice > static_cast<int>(members.size()))
        {
            std::cout << "选择无效\n";
            continue;
        }

        const auto& member =members[choice - 1];
        int64_t userId =member.value("userId", 0);
        std::string username = member.value("nickname",member.value("username", ""));
        groupMemberActionMenu(client,groupId,userId, username);
       }
    }
}
void groupMemberActionMenu(Client& client,int64_t groupId,int64_t userId,const std::string& username)
{
    while(true)
    {
        std::cout<< "\n========== 成员管理 ==========\n";
        std::cout << "成员: "<< username<< " (" << userId<< ")\n";
        std::cout << "1. 设置为管理员\n";
        std::cout << "2. 取消管理员\n";
        std::cout << "3. 移除成员\n";
        std::cout << "0. 返回\n";
        int op;
        std::cout << "请选择：";
        if(!(std::cin >> op))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            continue;
        }
        switch(op)
        {
        case 1:
            client.setGroupMemberRole(groupId,userId,GroupRole::Admin);
            return;

        case 2:
            client.setGroupMemberRole(  groupId,userId,GroupRole::Member);
            return;
        case 3:
            client.removeGroupMember(groupId,userId);

            return;

        case 0:
            return;

        default:
            std::cout << "选择无效\n";
            break;
        }
    }
}