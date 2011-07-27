
project(featuremaptest)

include_directories(../../src ${PROJECT_BINARY_DIR}/../../src)
INCLUDE(CheckTypeSize)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})

add_executable(nametabletest nametabletest.cpp
    ../../src/NameTable.cpp)

add_test(NAME nametabletest COMMAND $<TARGET_FILE:nametabletest>)
