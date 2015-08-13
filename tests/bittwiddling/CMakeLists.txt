CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
project(bits)
include(Graphite)
include_directories(${graphite2_core_SOURCE_DIR})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
    add_custom_target(${PROJECT_NAME}_copy_dll ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    add_dependencies(${PROJECT_NAME}_copy_dll graphite2 bits)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


add_executable(bits bits.cpp)
target_link_libraries(bits graphite2-base)

add_test(NAME bits COMMAND $<TARGET_FILE:bits>)
if (GRAPHITE2_ASAN)
    set_target_properties(bits PROPERTIES LINK_FLAGS "-fsanitize=address")
    set_property(TEST bits APPEND PROPERTY ENVIRONMENT "ASAN_SYMBOLIZER_PATH=${ASAN_SYMBOLIZER}")
endif (GRAPHITE2_ASAN)
