cmake_minimum_required(VERSION 3.16)

file(GLOB MINI_NET_SRC
    net/*.cpp
)

add_library(minimuduo STATIC
    ${MINI_NET_SRC}
)

target_include_directories(minimuduo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(minimuduo
    pthread
)