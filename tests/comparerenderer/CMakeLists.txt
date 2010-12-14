project(comparerenderer)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Freetype)
    find_package(PkgConfig)

    pkg_check_modules(GR silgraphite)
    pkg_check_modules(HB harfbuzz)
    pkg_check_modules(GLIB glib-2.0)
    # pkg_check_modules(GRNG graphite)

    if (${HB_FOUND})
        if (${FREETYPE_FOUND})
            add_definitions(-DHAVE_HARFBUZZNG)
        endif (${FREETYPE_FOUND})
    endif (${HB_FOUND})

    if (${GR_FOUND})
        add_definitions(-DHAVE_GRAPHITE)
        set(GR_EXTRA_SOURCE GrUtfTextSrc.cpp)
    endif (${GR_FOUND})
    set(RT_LDFLAGS "rt")

endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include_directories(../common ${PROJECT_SOURCE_DIR}/../../src ${FREETYPE_INCLUDE_DIRS} ${HB_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${GR_INCLUDE_DIRS})

file(GLOB PRIVATE_HEADERS *.h) 
set_source_files_properties(${PRIVATE_HEADERS} PROPERTIES HEADER_FILE_ONLY true)

add_executable(comparerenderer
    CompareRenderer.cpp
    ${GR_EXTRA_SOURCE}
    ${PRIVATE_HEADERS}
    )

# rt is for timing
target_link_libraries(comparerenderer graphite2 ${FREETYPE_LIBRARIES} ${GLIB_LD_FLAGS} ${HB_LDFLAGS} ${GR_LDFLAGS} ${RT_LDFLAGS})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	find_file(USP10_H NAMES Usp10.h)
	find_library(USP10_LIB NAMES Usp10.lib)
	if (NOT ${USP10_H} STREQUAL "USP10_H_NOTFOUND")
		add_definitions(-DHAVE_USP10)
	endif (NOT ${USP10_H} STREQUAL "USP10_H_NOTFOUND")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_custom_target(cr_copy_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    add_dependencies(cr_copy_dll graphite2 iconv comparerenderer)
endif  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

macro(comparetest TESTNAME FONTFILE TEXTFILE)
    if (${GR_FOUND})
    # comparision tests between old and new graphite
        add_test(NAME ${TESTNAME}_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${PROJECT_SOURCE_DIR}/../texts/${TEXTFILE} -f ${PROJECT_SOURCE_DIR}/../fonts/${FONTFILE} -s 12 -l ${PROJECT_BINARY_DIR}/${TESTNAME}_comparison.log -c -g -n)
        add_test(NAME ${TESTNAME}_cached_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${PROJECT_SOURCE_DIR}/../texts/${TEXTFILE} -f ${PROJECT_SOURCE_DIR}/../fonts/${FONTFILE} -s 12 -l ${PROJECT_BINARY_DIR}/${TESTNAME}_cached_comparison.log -c -g -n --seg-cache 4096)
    endif (${GR_FOUND})
endmacro(comparetest)

comparetest(charis charis_r_gr.ttf udhr_eng.txt)
comparetest(padauk Padauk.ttf my_HeadwordSyllables.txt)
comparetest(charis_yor charis_r_gr.ttf udhr_yor.txt)

# comparerenderer can also be used for font testing
install(TARGETS comparerenderer EXPORT comparerenderer RUNTIME DESTINATION bin)
