cmake_minimum_required(VERSION 3.8)
project(TcpClient)

set(CMAKE_CXX_STANDARD 17)
set(SHARED_DIR ../Shared)
include_directories(${SHARED_DIR})

set(SOURCE_FILES main.cpp API.h ClientWorker.h ClientWorker.cpp )
set(SOURCE_FILES ${SOURCE_FILES} ${SHARED_DIR}/Message.h ${SHARED_DIR}/Message.cpp ${SHARED_DIR}/SharedFunctions.h)
add_executable(TcpClient ${SOURCE_FILES})
