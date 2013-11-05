project(grlisttest)

include_directories(${graphite2_core_SOURCE_DIR})

add_executable(grlisttest grlisttest.cpp)
if (GRAPHITE2_ASAN)
    set_target_properties(grlisttest PROPERTIES LINK_FLAGS "-fsanitize=address")
endif (GRAPHITE2_ASAN)

add_test(NAME grlist COMMAND $<TARGET_FILE:grlisttest>)
