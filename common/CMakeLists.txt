cmake_minimum_required(VERSION 3.16)

find_package(OpenSSL REQUIRED)


file(GLOB_RECURSE COMMON_SRC
*.cpp
)


add_library(common STATIC
${COMMON_SRC}
)


target_include_directories(common PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(common PUBLIC
OpenSSL::SSL
OpenSSL::Crypto
)