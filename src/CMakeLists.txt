cmake_minimum_required(VERSION 3.11.0)
project(DoochiCore)

set (CMAKE_CXX_STANDARD 17)
# set (CMAKE_CXX_FLAGS "-Wall -Wextra")

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
  add_glob(${prefix}_sources ${common_path}/*.c ${common_path}/*.h)
endmacro()
#######################
## MACRO END
#######################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../include)

add_headers_and_sources(DoochiCore Analysis)
add_headers_and_sources(DoochiCore Util)
#add_headers_and_sources(DoochiCore Util/Pack)
add_headers_and_sources(DoochiCore Document)
add_headers_and_sources(DoochiCore Index)
add_headers_and_sources(DoochiCore Store)
#add_headers_and_sources(DoochiCore Codec)
add_library(DoochiCore SHARED ${DoochiCore_headers} ${DoochiCore_sources})

#add_subdirectory(Codec)
add_subdirectory(Analysis)
add_subdirectory(Util)
add_subdirectory(Document)
add_subdirectory(Index)
add_subdirectory(Store)
