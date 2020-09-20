cmake_minimum_required(VERSION 3.0)

project(EasyTCPUDPServerClientMain)

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")

# Select compiler and flags for CXX
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Werror")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

add_library(
  tcp_udp_srv_cli
  tcp_udp_srv_cli.cc)

# Add custom target to check BUILD type
add_custom_target(print_build_type COMMAND ${CMAKE_COMMAND} -E echo
                                           ${CMAKE_BUILD_TYPE})
