project(testing)

add_subdirectory(vm)
add_subdirectory(comparerenderer)

if (EXISTS ${PROJECT_SOURCE_DIR}/grlist)
add_subdirectory(grlist)
endif(EXISTS ${PROJECT_SOURCE_DIR}/grlist)

enable_testing()

macro(fonttest TESTNAME FONTFILE)
    if (EXISTS ${PROJECT_SOURCE_DIR}/standards/${TESTNAME}${CMAKE_SYSTEM_NAME}.log)
        set(PLATFORM_TEST_SUFFIX ${CMAKE_SYSTEM_NAME})
    endif (EXISTS ${PROJECT_SOURCE_DIR}/standards/${TESTNAME}${CMAKE_SYSTEM_NAME}.log)
    add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:grngfonttest> -log ${PROJECT_BINARY_DIR}/${TESTNAME}.log ${PROJECT_SOURCE_DIR}/fonts/${FONTFILE} -codes ${ARGN})
    add_test(NAME ${TESTNAME}Output COMMAND ${CMAKE_COMMAND} -E compare_files ${PROJECT_BINARY_DIR}/${TESTNAME}.log ${PROJECT_SOURCE_DIR}/standards/${TESTNAME}${PLATFORM_TEST_SUFFIX}.log)
    set_tests_properties(${TESTNAME}Output PROPERTIES DEPENDS ${TESTNAME})
    add_custom_target(${TESTNAME} COMMAND ${grngfonttest_BINARY_DIR}/${CMAKE_CFG_INTDIR}/grngfonttest${CMAKE_EXECUTABLE_SUFFIX} -trace ${TESTNAME}.xml ${PROJECT_SOURCE_DIR}/fonts/${FONTFILE} -codes ${ARGN})
    unset(PLATFORM_TEST_SUFFIX)
#    add_custom_target(${TESTNAME} DEPENDS ${TESTNAME}.xml)
endmacro(fonttest)

fonttest(padauk1 Padauk.ttf 1015 102F 100F 1039 100F 1031 1038)
fonttest(padauk2 Padauk.ttf 1000 103C 102D 102F)
fonttest(padauk3 Padauk.ttf 101e 1004 103a 1039 1001 103b 102d 102f 1004 103a 1038)
fonttest(padauk4 Padauk.ttf 1005 1000 1039 1000 1030)
fonttest(padauk5 Padauk.ttf 1000 103c 1031 102c 1004 1037 103a)
fonttest(padauk6 Padauk.ttf 1000 102D 1005 1039 1006 102C)
# padauk7 can cause an infinite loop, though the text is miss-spelt
fonttest(padauk7 Padauk.ttf 1017 1014 103c 103d 102f)
fonttest(charis1 charis_r_gr.ttf 0069 02E6 02E8 02E5)
fonttest(scher1 Scheherazadegr.ttf 0628 0628 064E 0644 064E 0654 0627 064E -rtl)

