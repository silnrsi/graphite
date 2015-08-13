project(endian)

include_directories(${graphite2_core_SOURCE_DIR})

add_executable(endiantest endiantest.cpp)
if (GRAPHITE2_ASAN)
    set_target_properties(endiantest PROPERTIES LINK_FLAGS "-fsanitize=address")
endif (GRAPHITE2_ASAN)

add_test(NAME endian COMMAND $<TARGET_FILE:endiantest> 10)
if (GRAPHITE2_ASAN)
    set_property(TEST endian APPEND PROPERTY ENVIRONMENT "ASAN_SYMBOLIZER_PATH=${ASAN_SYMBOLIZER}")
endif (GRAPHITE2_ASAN)
