project(grlisttest)

include_directories(${graphite2_core_SOURCE_DIR})

add_executable(grlisttest grlisttest.cpp)

add_test(NAME grlist COMMAND $<TARGET_FILE:grlisttest>)
