project(nametabletest)
include_directories(${graphite2_core_SOURCE_DIR})

add_executable(nametabletest nametabletest.cpp)
target_link_libraries(nametabletest graphite2-base)
if (GRAPHITE2_ASAN)
    set_target_properties(nametabletest PROPERTIES LINK_FLAGS "-fsanitize=address")
endif (GRAPHITE2_ASAN)

add_test(NAME nametabletest COMMAND $<TARGET_FILE:nametabletest>)
set_tests_properties(nametabletest PROPERTIES TIMEOUT 3)
