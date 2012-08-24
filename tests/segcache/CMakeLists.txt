CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
project(grsegcachetest)
include(Graphite)
include_directories(${graphite2_core_SOURCE_DIR})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
    add_custom_target(${PROJECT_NAME}_copy_dll ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    add_dependencies(${PROJECT_NAME}_copy_dll graphite2 grsegcachetest)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


set(S ${graphite2_core_SOURCE_DIR})
add_executable(grsegcachetest segcachetest.cpp)
set_target_properties(grsegcachetest PROPERTIES COMPILE_DEFINITIONS "GRAPHITE2_STATIC")
target_link_libraries(grsegcachetest graphite2 graphite2-segcache graphite2-base)
    
add_test(NAME grsegcachetest COMMAND $<TARGET_FILE:grsegcachetest> ${testing_SOURCE_DIR}/fonts/Padauk.ttf)
set_tests_properties(grsegcachetest PROPERTIES TIMEOUT 3)