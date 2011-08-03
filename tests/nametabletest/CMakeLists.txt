project(nametabletest)
include_directories(../../src)

target_objects(OBJS graphite2 NameTable.cpp)
add_executable(nametabletest nametabletest.cpp ${OBJS})
add_dependencies(nametabletest graphite2)

add_test(NAME nametabletest COMMAND $<TARGET_FILE:nametabletest>)
