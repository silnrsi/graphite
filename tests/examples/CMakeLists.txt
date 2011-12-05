
project(testexamples)

include_directories(../../src)
INCLUDE(CheckTypeSize)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
    add_custom_target(${PROJECT_NAME}_copy_dll ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
	add_dependencies(${PROJECT_NAME}_copy_dll graphite2 iconv simple features clusters linebreak)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

macro(test_example TESTNAME SRCFILE)
    add_executable(${TESTNAME} ${SRCFILE})
    set_target_properties(${TESTNAME} PROPERTIES LINKER_LANGUAGE C)
    target_link_libraries(${TESTNAME} graphite2)

    add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:${TESTNAME}> ${ARGN})
endmacro(test_example)

test_example(simple simple.c ${testing_SOURCE_DIR}/fonts/Padauk.ttf "Hello World!")
test_example(features features.c ${testing_SOURCE_DIR}/fonts/Padauk.ttf)
test_example(clusters cluster.c ${testing_SOURCE_DIR}/fonts/Padauk.ttf "စက္ခုန္ဒြေ")
test_example(linebreak linebreak.c ${testing_SOURCE_DIR}/fonts/charis_r_gr.ttf 120 "This is a long test line that goes on and on and on")
