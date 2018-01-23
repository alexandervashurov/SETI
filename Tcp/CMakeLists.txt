cmake_minimum_required(VERSION 3.8)
project(Tcp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /std:c++17)

set(SERVER_SRC API.h CTcpServer.h CTcpServer.cpp ServerWorker.h ServerWorker.cpp stdinc.h)
add_executable(server ${SERVER_SRC} main.cpp)
target_link_libraries(server ws2_32)