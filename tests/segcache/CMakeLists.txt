project(grsegcachetest)

include_directories(../../src)

# TtfUtil isn't exported, so we need to link it twice, ugly, but this is only a test
add_executable(grsegcachetest segcachetest.cpp ../../src/TtfUtil.cpp)

target_link_libraries(grsegcachetest graphiteng)

add_test(NAME grsegcachetest COMMAND $<TARGET_FILE:grsegcachetest> ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf)
