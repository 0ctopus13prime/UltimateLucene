cmake_minimum_required(VERSION 2.8.12)
project(DoochiCore)

set(CMAKE_CXX_FLAGS "-std=c++11")

#######################
## MACRO START
#######################
macro(add_glob cur_list)
  file(GLOB __tmp RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN})
  list(APPEND ${cur_list} ${__tmp})
endmacro()

macro(add_headers_and_sources prefix common_path)
  add_glob(${prefix}_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${common_path}/*.h)
  add_glob(${prefix}_sources ${common_path}/*.cpp ${common_path}/*.h)
endmacro()
#######################
## MACRO END
#######################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_headers_and_sources(DoochiCore Analysis)
add_headers_and_sources(DoochiCore Util)
add_library(DoochiCore SHARED ${DoochiCore_headers} ${DoochiCore_sources})

add_subdirectory(Analysis)
add_subdirectory(Util)
