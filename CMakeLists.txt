cmake_minimum_required(VERSION 3.10)

project(udpz)

add_executable(a udp_a.cpp)
add_executable(b udp_b.cpp)
add_executable(kcpa kcpa.cpp)

install_targets(a b)