CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
project(utftest)
include(Graphite)
include_directories(../../src)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


target_objects(OBJS graphite2)
add_executable(utftest utftest.cpp ${OBJS})
add_dependencies(utftest graphite2)

add_test(NAME utftest COMMAND $<TARGET_FILE:utftest>)
