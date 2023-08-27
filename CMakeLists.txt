cmake_minimum_required(VERSION 3.25)

project(hello CXX)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "Set lto")
set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system json)
find_package(LibDataChannel CONFIG REQUIRED)
find_package(Threads)

add_executable(hello main.cpp)

target_link_libraries(hello PRIVATE Boost::system Boost::json Threads::Threads LibDataChannel::LibDataChannelStatic)
