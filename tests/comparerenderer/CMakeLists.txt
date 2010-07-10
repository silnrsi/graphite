project(comparerenderer)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Freetype)
    find_package(PkgConfig)

    pkg_check_modules(GR silgraphite)
    pkg_check_modules(HB harfbuzz)
    pkg_check_modules(GLIB glib-2.0)
    # pkg_check_modules(GRNG graphite)

    if (${HB_FOUND} AND ${FREETYPE_FOUND})
        add_definitions(-DHAVE_HARFBUZZNG)
    endif (${HB_FOUND} AND ${FREETYPE_FOUND})

    if (${GR_FOUND})
        add_definitions(-DHAVE_GRAPHITE)
    endif (${GR_FOUND})

endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include_directories(../common icule ${FREETYPE_INCLUDE_DIRS} ${HB_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${GR_INCLUDE_DIRS})

add_executable(comparerenderer
    CompareRenderer.cpp
    GrUtfTextSrc.cpp
    )

# rt is for timing
target_link_libraries(comparerenderer graphiteng ${FREETYPE_LIBRARIES} ${GLIB_LD_FLAGS} ${HB_LDFLAGS} ${GR_LDFLAGS} rt)

if (${GR_FOUND})
    # comparision tests between old and new graphite
    # this file is fine as a test for Latin, but beware tabs in the file may cause discrepancies
    add_test(NAME charis_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${PROJECT_SOURCE_DIR}/CMakeLists.txt -f ${PROJECT_SOURCE_DIR}/../fonts/charis_r_gr.ttf -s 12 -l ${PROJECT_BINARY_DIR}/charis_comparision.log -c -g -n)
    add_test(NAME padauk_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${PROJECT_SOURCE_DIR}/../texts/my_HeadwordSyllables.txt -f ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf -s 12 -l ${PROJECT_BINARY_DIR}/padauk_comparision.log -c -g -n)

endif (${GR_FOUND})
