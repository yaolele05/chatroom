cmake_minimum_required(VERSION 3.16)

enable_testing()


# Login测试
add_executable(test_login
    testlogin.cpp)

target_link_libraries(test_login
    chatserver_lib)


# UserModel测试
add_executable(test_usermodel
    testusermodel.cpp)

target_link_libraries(test_usermodel
    chatserver_lib)


add_executable(test_message
    testmessage.cpp)


target_link_libraries(test_message
    chatserver_lib)

add_executable(test_group_message
    testgroupmessage.cpp)

target_link_libraries(test_group_message
    chatserver_lib)

add_executable(test_friend
    testfriend.cpp)


target_link_libraries(test_friend
    chatserver_lib)

add_executable(test_group
   testgroup.cpp)

target_link_libraries(test_group
   chatserver_lib)

add_executable(test_offlinemessage
   testofflinemessage.cpp)
   
target_link_libraries(test_offlinemessage
   chatserver_lib)