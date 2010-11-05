project(grsegcachetest)

include_directories(../../src)

add_executable(grsegcachetest segcachetest.cpp)

target_link_libraries(grsegcachetest graphiteng)

add_test(NAME grsegcachetest COMMAND $<TARGET_FILE:grsegcachetest> ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf)
