cmake_minimum_required(VERSION 3.0)

project(EasyTCPUDPServerClientSamples)

include_directories(../src)

# Select compiler and flags for CXX
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "-std=c++17")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

add_executable(
  server_tcpip
  server_tcpip.cc)

target_link_libraries(server_tcpip pthread tcp_udp_srv_cli)

add_executable(
  client_tcpip
  client_tcpip.cc)

target_link_libraries(client_tcpip pthread tcp_udp_srv_cli)
