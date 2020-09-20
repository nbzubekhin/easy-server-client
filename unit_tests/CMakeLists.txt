cmake_minimum_required(VERSION 3.0)

project(EasyTCPUDPServerClientTests)

# Setup testing
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
if(NOT "$ENV{GMOCK_HOME}" STREQUAL "")
  include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
  link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild)
else()
  include_directories(/usr/include/gtest/include)
  link_directories(/usr/lib64/)
endif()
add_definitions(-std=c++17)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -Wall")

set(sources main.cc tcp_udp_srv_cli_test.cc string_split_test.cc)

add_executable(runUnitTest ${sources})
target_link_libraries(
  runUnitTest
  gtest
  gtest_main
  tcp_udp_srv_cli
  pthread)

set(UNIT_TEST runUnitTest)
add_custom_command(
  TARGET ${UNIT_TEST}
  COMMENT
    "Running tests..."
  POST_BUILD
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST})
