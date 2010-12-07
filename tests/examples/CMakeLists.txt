
project(testexamples)

include_directories(../../src)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

macro(test_example TESTNAME SRCFILE)
    add_executable(${TESTNAME} ${SRCFILE})
    add_dependencies(${TESTNAME} graphiteng)
    set_target_properties(${TESTNAME} PROPERTIES LINK_FLAGS "-L ${PROJECT_BINARY_DIR}/../../src" LINKER_LANGUAGE C)
    target_link_libraries(${TESTNAME} -lgraphiteng)

    add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:${TESTNAME}> ${ARGN})
    set_tests_properties(${TESTNAME} PROPERTIES ENVIRONMENT LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}/../../src)
endmacro(test_example)

test_example(simple simple.c ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf "Hello World!")
test_example(features features.c ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf)
