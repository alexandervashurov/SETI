cmake_minimum_required(VERSION 3.8)
project(TcpClient)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp API.h ClientWorker.h ClientWorker.cpp)
add_executable(TcpClient ${SOURCE_FILES})
