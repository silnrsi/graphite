project(comparerenderer)

find_path(ICU_INCLUDE NAMES unicode/utypes.h 
	PATHS /usr/local/include /usr/include ${PROJECT_SOURCE_DIR}/../../../icu/source/common
	DOC "Include directory for the ICU library")
find_path(ICU_LAYOUT_INCLUDE NAMES layout/LETypes.h
	PATHS /usr/local/include /usr/include ${PROJECT_SOURCE_DIR}/../../../icu/source
	DOC "Include directory for the ICU layout engine")
find_path(ICU_LIBRARY_PATH NAMES icuuc${CMAKE_LINK_LIBRARY_SUFFIX} 
	${CMAKE_SHARED_LIBRARY_PREFIX}icuuc${CMAKE_SHARED_LIBRARY_SUFFIX} cygicuuc cygicuuc32
	PATHS /usr/local/lib /usr/lib ${PROJECT_SOURCE_DIR}/../../../icu/source/lib
	DOC "Libraries to link against for the common parts of ICU")

set(ICULE)
if (${ICU_INCLUDE} STREQUAL "ICU_INCLUDE-NOTFOUND")
	set(ICU_FOUND 0)
	set(ICU_INCLUDE_DIRS) 
	set(ICU_LDFLAGS)
	set(ICU_LIBRARY_PATH)
else (${ICU_INCLUDE} STREQUAL "ICU_INCLUDE-NOTFOUND")
    set(ICU_FOUND 1)
    file(GLOB ICULE icule/*.cpp icule/*.h)
    add_definitions(-DHAVE_ICU)
    set(ICU_INCLUDE_DIRS "${ICU_INCLUDE}" "${ICU_LAYOUT_INCLUDE}")
    link_directories(${ICU_LIBRARY_PATH})
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set(ICU_LDFLAGS icule icuuc icudt)
    else (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set(ICU_LDFLAGS icule icuuc icudata)
    endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
endif (${ICU_INCLUDE} STREQUAL "ICU_INCLUDE-NOTFOUND")

#find_path(HB1_INCLUDE NAMES "harfbuzz-shaper.h"
#        PATHS /usr/local/include/harfbuzz-1 /usr/include/harfbuzz-1
#        DOC "Include directory for harfbuzz")
#if (NOT ${HB1_INCLUDE} STREQUAL "")
#    add_definitions(-DHAVE_HARFBUZZ)
#    set(HB1_LDFLAGS "-L${HB1_INCLUDE}/../../lib -lharfbuzz-1")
#endif (${HB1_INCLUDE})

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

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	find_path(GR_INCLUDE graphite/GrClient.h PATHS ENV SILGRAPHITE_HOME ${PROJECT_SOURCE_DIR}/../../../silgraphite-2.3.1 ${PROJECT_SOURCE_DIR}/../../../silgraphite-2.4.0 ${GRAPHITE_INSTALLED_PATH} ${PROJECT_SOURCE_DIR}/../../../graphite-trunk PATH_SUFFIXES engine/include include)
    find_library(GR_DEBUG_LIBRARY graphite PATHS ENV SILGRAPHITE_HOME ${PROJECT_SOURCE_DIR}/../../../silgraphite-2.3.1 ${PROJECT_SOURCE_DIR}/../../../silgraphite-2.4.0 ${GR_GRAPHITE_DEBUG} ${PROJECT_SOURCE_DIR}/../../../graphite-trunk PATH_SUFFIXES engine/debug vs9/engine/src/Debug build/engine/src/Debug)
    find_library(GR_RELEASE_LIBRARY graphite PATHS ENV SILGRAPHITE_HOME ${PROJECT_SOURCE_DIR}/../../../silgraphite-2.3.1 ${PROJECT_SOURCE_DIR}/../../../silgraphite-2.4.0 ${GR_GRAPHITE_RELEASE} ${PROJECT_SOURCE_DIR}/../../../graphite-trunk PATH_SUFFIXES engine/release vs9/engine/src/Release build/engine/src/Release)
    if (NOT GR_INCLUDE STREQUAL GR_INCLUDE-NOTFOUND)
		set(GR_FOUND 1)
		add_definitions(-DHAVE_GRAPHITE)
		set(GR_INCLUDE_DIRS ${GR_INCLUDE})
        set(GR_EXTRA_SOURCE GrUtfTextSrc.cpp)
    endif (NOT GR_INCLUDE STREQUAL GR_INCLUDE-NOTFOUND)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

include_directories(../common ${PROJECT_SOURCE_DIR}/../../src ${FREETYPE_INCLUDE_DIRS} ${HB_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${GR_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS} ${HB1_INCLUDE})

file(GLOB PRIVATE_HEADERS *.h) 
set_source_files_properties(${PRIVATE_HEADERS} PROPERTIES HEADER_FILE_ONLY true)

add_executable(comparerenderer
    CompareRenderer.cpp
    ${GR_EXTRA_SOURCE}
    ${ICULE}
    ${PRIVATE_HEADERS}
    )

# rt is for timing
target_link_libraries(comparerenderer graphite2-base graphite2 ${FREETYPE_LIBRARIES} ${GLIB_LD_FLAGS} ${HB_LDFLAGS} ${HB1_LDFLAGS} ${GR_LDFLAGS} ${ICU_LDFLAGS} ${RT_LDFLAGS})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	find_file(USP10_H NAMES Usp10.h)
	find_library(USP10_LIB NAMES Usp10.lib)
	if (NOT ${USP10_H} STREQUAL "USP10_H_NOTFOUND")
		add_definitions(-DHAVE_USP10)
	endif (NOT ${USP10_H} STREQUAL "USP10_H_NOTFOUND")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
    add_custom_target(${PROJECT_NAME}_copy_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    add_dependencies(${PROJECT_NAME}_copy_dll graphite2 iconv comparerenderer)
    if (${GR_FOUND})
        target_link_libraries(comparerenderer debug ${GR_DEBUG_LIBRARY})
        target_link_libraries(comparerenderer optimized ${GR_RELEASE_LIBRARY})
    endif (${GR_FOUND})
    if (NOT ${ICU_LIBRARY_PATH} STREQUAL "ICU_LIBRARY_PATH-NOTFOUND")
        file(GLOB ICU_DLLS ${ICU_LIBRARY_PATH}/*.dll)
        foreach(ICU_DLL ${ICU_DLLS})
	        add_custom_command(TARGET ${PROJECT_NAME}_copy_dll POST_BUILD
		        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ICU_DLL} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})        
	    endforeach(ICU_DLL ${ICU_DLLS})
    endif (NOT ${ICU_LIBRARY_PATH} STREQUAL "ICU_LIBRARY_PATH-NOTFOUND")

endif  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

macro(comparetest TESTNAME FONTFILE TEXTFILE)
    if (${GR_FOUND})
    # comparision tests between old and new graphite
        add_test(NAME ${TESTNAME}_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${testing_SOURCE_DIR}/texts/${TEXTFILE} -f ${testing_SOURCE_DIR}/fonts/${FONTFILE} -s 12 -l ${PROJECT_BINARY_DIR}/${TESTNAME}_comparison.log -c -g -n --tolerance 0.0001 ${ARGN})
        add_test(NAME ${TESTNAME}_cached_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${testing_SOURCE_DIR}/texts/${TEXTFILE} -f ${testing_SOURCE_DIR}/fonts/${FONTFILE} -s 12 -l ${PROJECT_BINARY_DIR}/${TESTNAME}_cached_comparison.log -c -g -n --seg-cache 4096 --tolerance 0.0001 ${ARGN})
        set_tests_properties(${TESTNAME}_comparison PROPERTIES TIMEOUT 3)
        set_tests_properties(${TESTNAME}_cached_comparison PROPERTIES TIMEOUT 3)
    endif (${GR_FOUND})
endmacro(comparetest)

comparetest(charis charis_r_gr.ttf udhr_eng.txt)
comparetest(padauk Padauk.ttf my_HeadwordSyllables.txt)
comparetest(padauk_ulon Padauk.ttf my_HeadwordSyllables.txt --features ulon=1)
comparetest(charis_yor charis_r_gr.ttf udhr_yor.txt)
comparetest(piglatin PigLatinBenchmark_v3.ttf udhr_eng.txt)
# silgraphite and gr2 bidi algorithms don't quite agree enough
# comparetest(scheherazade_arb Scheherazadegr.ttf udhr_arb.txt --rtl)
comparetest(annapurna_nep Annapurnarc2.ttf udhr_nep.txt)

# comparerenderer can also be used for font testing
install(TARGETS comparerenderer EXPORT comparerenderer RUNTIME DESTINATION bin)
