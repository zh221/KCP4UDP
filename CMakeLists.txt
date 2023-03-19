cmake_minimum_required(VERSION 3.10)

project(udpz)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 

add_executable(a udp_a.cpp)
add_executable(b udp_b.cpp)

install_targets(a b)