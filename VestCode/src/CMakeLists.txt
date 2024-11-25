# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

if(PROJECT STREQUAL "vest1")
  list(FILTER app_sources EXCLUDE REGEX ".*blaster_main.cpp")
endif()

if(PROJECT STREQUAL "vest2")
  list(FILTER app_sources EXCLUDE REGEX ".*blaster_main.cpp")
endif()

if(PROJECT STREQUAL "blaster1")
  list(FILTER app_sources EXCLUDE REGEX ".*vest_main.cpp")
endif()

if(PROJECT STREQUAL "blaster2")
  list(FILTER app_sources EXCLUDE REGEX ".*vest_main.cpp")
endif()

idf_component_register(SRCS ${app_sources})
