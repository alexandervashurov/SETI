cmake_minimum_required(VERSION 3.8)
project(UdpServer)

set(CMAKE_CXX_STANDARD 17)
set(SHARED_DIR ../Shared)
include_directories(${SHARED_DIR})

set(SERVER_SRC API.h CTcpServer.h CTcpServer.cpp ServerWorker.h ServerWorker.cpp stdinc.h)
set(SERVER_SRC ${SERVER_SRC} ${SHARED_DIR}/Message.h ${SHARED_DIR}/Message.cpp ${SHARED_DIR}/SharedFunctions.h)
add_executable(server ${SERVER_SRC} main.cpp)
target_link_libraries(server pthread)
