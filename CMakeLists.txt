cmake_minimum_required(VERSION 3.16)

project(ChatRoom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(common)
add_subdirectory(minimuduo)
add_subdirectory(server)
add_subdirectory(client)
