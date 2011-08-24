project(endian)

include_directories(../../src)

add_executable(endiantest endiantest.cpp)

add_test(NAME endian COMMAND $<TARGET_FILE:endiantest> 10)
