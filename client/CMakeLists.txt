cmake_minimum_required(VERSION 3.16)


find_package(OpenSSL REQUIRED)


file(GLOB_RECURSE CLIENT_SRC
*.cpp
)


add_executable(chatclient
${CLIENT_SRC}
)


target_include_directories(chatclient PUBLIC

${CMAKE_CURRENT_SOURCE_DIR}

${PROJECT_SOURCE_DIR}/common

${PROJECT_SOURCE_DIR}/minimuduo

)


target_link_libraries(chatclient PUBLIC

minimuduo

common

OpenSSL::SSL

OpenSSL::Crypto

pthread

)