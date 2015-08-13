project(grlisttest)

include_directories(${graphite2_core_SOURCE_DIR})

set(S ${graphite2_core_SOURCE_DIR})

add_executable(grlisttest grlisttest.cpp)
#add_executable(intervalsettest intervalsettest.cpp)

if (GRAPHITE2_ASAN)
    set_target_properties(grlisttest PROPERTIES LINK_FLAGS "-fsanitize=address")
#    set_target_properties(intervalsettest PROPERTIES LINK_FLAGS "-fsanitize=address")
endif (GRAPHITE2_ASAN)

#target_link_libraries(intervalsettest graphite2 graphite2-base)
add_test(NAME grlist COMMAND $<TARGET_FILE:grlisttest>)
#add_test(NAME intervalset COMMAND $<TARGET_FILE:intervalsettest>)
if (GRAPHITE2_ASAN)
    set_property(TEST grlist APPEND PROPERTY ENVIRONMENT "ASAN_SYMBOLIZER_PATH=${ASAN_SYMBOLIZER}")
#    set_property(TEST intervalset APPEND PROPERTY ENVIRONMENT "ASAN_SYMBOLIZER_PATH=${ASAN_SYMBOLIZER}")
endif (GRAPHITE2_ASAN)
