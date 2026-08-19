cmake_minimum_required(VERSION 3.16)


find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

find_library(MYSQLCLIENT_LIB
    mysqlclient
    REQUIRED
)
find_library(HIREDIS_LIB
    hiredis
    REQUIRED
)
file(GLOB_RECURSE SERVER_SRC
    *.cpp
)
list(REMOVE_ITEM SERVER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)


add_library(chatserver_lib STATIC
    ${SERVER_SRC}
)

target_include_directories(chatserver_lib PUBLIC

    ${CMAKE_CURRENT_SOURCE_DIR}

    ${PROJECT_SOURCE_DIR}/minimuduo

    /usr/include/mysql

    /usr/local/include/hiredis
)
target_link_libraries(chatserver_lib PUBLIC

    minimuduo

    common

    ${MYSQLCLIENT_LIB}

    ${HIREDIS_LIB}

    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
    CURL::libcurl
)


add_executable(chatserver
    main.cpp
)
target_link_libraries(chatserver

    chatserver_lib

)
target_compile_options(chatserver PRIVATE
    -fsanitize=address
    -fno-omit-frame-pointer
)

target_link_options(chatserver PRIVATE
    -fsanitize=address
)