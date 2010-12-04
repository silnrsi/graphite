
project(testexamples)

include_directories(../../src)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_executable(simple simple simple.c)
add_dependencies(simple graphiteng)
add_definitions(simple -I ${PROJECT_SOURCE_DIR}/../../include)
set_target_properties(simple PROPERTIES LINK_FLAGS "-L ${PROJECT_BINARY_DIR}/../../src" LINKER_LANGUAGE C)
target_link_libraries(simple -lgraphiteng)

add_test(NAME simple COMMAND $<TARGET_FILE:simple> ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf "Hello World")
set_tests_properties(simple PROPERTIES ENVIRONMENT LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}/../../src)
