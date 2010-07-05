project(testing)

add_subdirectory(vm)
enable_testing()

macro(fonttest TESTNAME FONTFILE)
    add_test(NAME ${TESTNAME} COMMAND ${PROJECT_BINARY_DIR}/../grngfonttest/grngfonttest ${OPTS} -log ${PROJECT_BINARY_DIR}/${TESTNAME}.log ${PROJECT_SOURCE_DIR}/fonts/${FONTFILE} -codes ${ARGN})
    add_test(NAME ${TESTNAME}Output COMMAND ${CMAKE_COMMAND} -E compare_files ${PROJECT_BINARY_DIR}/${TESTNAME}.log ${PROJECT_SOURCE_DIR}/standards/${TESTNAME}.log)
    set_tests_properties(${TESTNAME}Output PROPERTIES DEPENDS ${TESTNAME})
    add_custom_command(OUTPUT ${TESTNAME}.xml COMMAND ${PROJECT_BINARY_DIR}/../grngfonttest/grngfonttest -trace ${TESTNAME}.xml ${PROJECT_SOURCE_DIR}/fonts/${FONTFILE} -codes ${ARGN})
    add_custom_target(${TESTNAME} DEPENDS ${TESTNAME}.xml)
endmacro(fonttest)

fonttest(padauk1 Padauk.ttf 1000 103C 1031 102C)
fonttest(padauk2 Padauk.ttf 1000 103C 102D 102F)
fonttest(charis1 charis_r_gr.ttf 0069 02E6 02E8 02E5)
fonttest(scher1 Scheherazadegr.ttf 0628 0628 064E 0644 064E 0654 0627 064E -rtl)

