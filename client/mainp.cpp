#include "client.h"
#include <iostream>
#include "../minimuduo/net/eventloop.h"
#include <thread>
#include <limits>
#include <chrono>
void loginMenu(Client& client);
void chatMenu(Client& client);
/*

int main()
{

    EventLoop loop;
    Client client(&loop);
    if(!client.connect("127.0.0.1",8888))
    {
        std::cout<<"connect failed\n";
        return -1;
    }


    
    //菜单线程
    std::thread menuThread([&client]()
        {
            loginMenu(client);
        }
    );

  
    //Reactor线程
   std::cout<<"before loop"<<std::endl;
    loop.loop();
    std::cout<<"after loop"<<std::endl;
    menuThread.join();
    return 0;
}
void chatMenu(Client& client)
{
    while(client.isLogin())
    {
        std::cout<<R"(

======== ChatRoom ========

1. 私聊
2. 添加好友
3. 删除好友
4.好友列表
5.创建群
6.加群
7.退群
8.群列表
9.群聊天
10.发送文件
11.注销
12.查看私聊历史
13.查看群聊历史
0.退出

==========================

)";
        int op;
        std::cout<<"请选择：";
    if(!(std::cin>>op))
    {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    continue;
     }
    switch(op)
    {
    case 1:
    {
    uint32_t id=0;
    std::string text;

    std::cout<<"friend:";
    std::cin>>id;
    std::cin.ignore();

    std::getline(std::cin,text);
    client.privateChat(id,text);
    break;
   }

   case 2:
  {
  uint32_t id=0;
  std::cout<<"Friendid:";
  if(!(std::cin>>id))
  {
     std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
  }
  std::cout<<"input friendId:"<<id<<std::endl;
  client.addFriend(id);
  break;
   }
 
   case 3:
    {
        uint32_t id=0;
        std::cout<<"Friendid:";
        if(!(std::cin>>id))
      {
       std::cin.clear();
       std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
      }
        
       std::cout<<"delete friendId=" <<id<<std::endl;
        client.deleteFriend(id);
        break;
    }
    case 4:
    {
      
        client.friendList();
        break;
        
    }
 case 5:
   {
      std::string groupName;
    std::string description;

    std::cout<<"group name:";
    std::cin>>groupName;

    std::cout<<"description:";
    std::cin>>description;

    client.createGroup(groupName, description);

   }
   case 6:
  {
   uint32_t gid;
   std::cout<<"输入要加入的groupid:";
   std::cin>>gid;
   client.joinGroup(gid);
   break;
  }
   case 7:
  {
   uint32_t gid;
   std::cout<<"请输入要退出的groupid:";
    std::cin>>gid;
  client.leaveGroup(gid);
   break;
   }
   case 8:
  {
    client.groupList();
    break;
  }
   case 9:
   {
    uint32_t gid;
    std::string text;
    std::cout<<"请选择群id:";
    std::cin>>gid;
    std::cin.ignore();
    std::getline(std::cin,text);
     client.groupChat(gid,text);
     break;
   }
  case 10:
   {
    int type;
   

    std::cout<<"1.私聊文件\n"<<"2.群文件\n";
    
    std::cin >> type;
    if(type==1)
    {

         uint32_t uid;
    std::string filename;
    std::cout << "请输入接收者ID：";
      std::cin>>uid;
    std::cout << "请输入文件路径：";
  
      std::cin >> filename;
      client.sendPrivateFile(uid, filename);

    
    }
    if(type==2)
    {
         uint32_t gid;
    std::string filename;
    std::cout << "请输入接收群ID：";
    std::cin>>gid;
    std::cout << "请输入文件路径：";
    std::cin >> filename;
      client.sendGroupFile(gid, filename);

    }

    break;
   }
  
   case 11:
   {
   client.logout();
   return;
   }

   case 12:
   {
    uint32_t uid;
    std::cout<<"好友ID:";
    std::cin>>uid;
    client.privateHistory(uid);
    break;

   } 


    case 13:
    {

    uint32_t gid;
    std::cout<<"群ID:";
    std::cin>>gid;

    client.groupHistory(gid);
    break;

     }

    case 0:
    exit(0);
    }
   
    }
}
void loginMenu(Client& client)
{
    while(true)
    {
        std::cout<<R"(

======== ChatRoom ========

1. 登陆

2. 注册

0. 退出

==========================

)";

        int op;
       
        if(!(std::cin>>op))
      {
      std::cin.clear();
      std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(),
        '\n');

      continue;
      }
        switch(op)
        {
            case 1:
        {
                std::string name;
                std::string pwd;

                std::cout<<"username:";
                std::cin>>name;

                std::cout<<"password:";
                std::cin>>pwd;

                client.login(name,pwd);
                if(client.waitLoginResult())
                 {
                  chatMenu(client);
                  }

                break;
       }

         case 2:
         {std::string user;
        std::string pwd;
        std::cout<<"username:";
        std::cin>>user;
        std::cout<<"password:";
        std::cin>>pwd;
       client.registerUser(user,pwd);
         break;
        }
        case 0:
        {
        client.disconnect();
        client.quit();
         return;
         }

        if(client.isLogin())
        {
            chatMenu(client);
        }
    }
  }
}*/