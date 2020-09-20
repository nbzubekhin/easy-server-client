cmake_minimum_required(VERSION 3.0)

project(EasyTCPUDPServerClient)

option(BUILD_SOURCES "Enable building sources" ON)
option(BUILD_UNIT_TESTS "Enable building unit tests" OFF)
option(BUILD_SAMPLES "Enable building samples" OFF)

if(BUILD_UNIT_TESTS)
  add_subdirectory(unit_tests)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

if(BUILD_SOURCES)
  add_subdirectory(src)
endif()
