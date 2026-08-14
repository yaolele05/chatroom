 #include "client.h"
#include <iostream>
#include "../minimuduo/net/eventloop.h"
#include <thread>
#include <limits>
#include <chrono>
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
int main()
{
    EventLoop loop;
    Client client(&loop);
    if(!client.connect("127.0.0.1",8888))
    {
        std::cout<<"connection failed\n";
        return -1;
    }
    std:: thread menuThread([&client](){
        loginMenu(client);
    });

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
8.注销

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
            client.logout();
            return;
           }
       
           case 0:
           client.disconnect();
           client.quit();
           return;
           
           default:
           std::cout<<"选择无效\n";
           break;
            

        }
    }
}
void friendMenu(Client& client)
{
    while(true)
    {
  
    std::cout << "\n========== 好友 ==========\n";
       client.friendList();
  
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
    ///这里要等接收完
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
     while(true)
    {
     std::cout<<R"(
==========================
1. 聊天
2.查看历史聊天记录
3.发送文件
4.待接收文件

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
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
              std::string text;
            std::getline(std::cin,text);
           client.privateChat(friendid,text);
            break;
          }
          case 2:
          {
            client.privateHistory(friendid);
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

        //这里要等接收完
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
              std::string text;
            std::getline(std::cin,text);
           client.groupChat(groupid,text);
            break;
          }
          case 2:
          {
           historyMenu(client);
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
           case 0:
          {
            return;

          }
          default:
           {
            break;
           }
          
        }
    }
  

    return;
}
void receiveFriendFileMenu(Client& client,int friendid)
{
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
            std::cout<< index<< ". "<< file.filename<< "       "<< file.filesize / 1024 / 1024<< " MB\n";
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
            std::cout<< index<< ". "<< file.filename<< "       "<< file.filesize / 1024 / 1024<< " MB\n";
            ++index;
        }

        std::cout << "0. 返回\n";

        int choice;
        std::cout << "\n请选择：<<std::flush";
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
    while(true)
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