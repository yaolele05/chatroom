cmake_minimum_required(VERSION 3.16)

project(chatserver)


file(GLOB_RECURSE SERVER_SRC
    CONFIGURE_DEPENDS
    *.cpp
)


add_executable(chatserver
    ${SERVER_SRC}
)


target_include_directories(chatserver
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/business
    ${CMAKE_CURRENT_SOURCE_DIR}/database
    ${CMAKE_CURRENT_SOURCE_DIR}/session
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)


target_link_libraries(chatserver
PRIVATE
    minimuduo
)