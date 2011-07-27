project(grlisttest)

include_directories(../../src ${PROJECT_BINARY_DIR}/../../src)

add_executable(grlisttest grlisttest.cpp)

add_test(NAME grlist COMMAND $<TARGET_FILE:grlisttest>)
