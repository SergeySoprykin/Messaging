cmake_minimum_required(VERSION 3.10)
project(NetworkMessageService)

find_package(Boost REQUIRED COMPONENTS system filesystem serialization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
endif()

set(SOURCES
    src/server.cpp src/client.cpp src/connection.cpp
)

include_directories(./include)

add_library(server_lib ${SOURCES})
target_link_libraries(server_lib Boost::system Boost::filesystem Boost::serialization pthread)

add_executable(server src/test_server.cpp)
target_link_libraries(server server_lib Boost::system Boost::filesystem Boost::serialization pthread)

add_executable(client src/test_client.cpp)
target_link_libraries(client server_lib Boost::system Boost::filesystem Boost::serialization pthread)


