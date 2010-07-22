project(grlisttest)

include_directories(../../src)

add_executable(grlisttest grlisttest.cpp)

add_test(NAME grlist COMMAND $<TARGET_FILE:grlisttest>)
