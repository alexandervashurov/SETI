cmake_minimum_required(VERSION 3.8)
project(UdpClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /std:c++17)

set(SHARED_DIR ../Shared)
include_directories(${SHARED_DIR})

set(SERVER_SRC API.h ClientWorker.cpp ClientWorker.h)
set(SERVER_SRC ${SERVER_SRC} ${SHARED_DIR}/Message.h ${SHARED_DIR}/Message.cpp ${SHARED_DIR}/SharedFunctions.h)
add_executable(client ${SERVER_SRC} main.cpp)
target_link_libraries(client ws2_32)
