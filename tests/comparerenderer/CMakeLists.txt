project(comparerenderer)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(Freetype)
    find_package(PkgConfig)

    pkg_check_modules(GR silgraphite)
    pkg_check_modules(HB harfbuzz)
    pkg_check_modules(GLIB glib-2.0)
    pkg_check_modules(GRNG graphite)

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

include_directories(../common ${FREETYPE_INCLUDE_DIRS} ${HB_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${GR_INCLUDE_DIRS})

add_executable(comparerenderer
    CompareRenderer.cpp
    ${GR_EXTRA_SOURCE}
    )

# rt is for timing
target_link_libraries(comparerenderer graphiteng ${FREETYPE_LIBRARIES} ${GLIB_LD_FLAGS} ${HB_LDFLAGS} ${GR_LDFLAGS} ${RT_LDFLAGS})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_custom_target(cr_copy_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphiteng_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphiteng${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    add_dependencies(cr_copy_dll graphiteng iconv comparerenderer)
endif  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

if (${GR_FOUND})
    # comparision tests between old and new graphite
    # this file is fine as a test for Latin, but beware tabs in the file may cause discrepancies
    add_test(NAME charis_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${PROJECT_SOURCE_DIR}/../texts/udhr_eng.txt -f ${PROJECT_SOURCE_DIR}/../fonts/charis_r_gr.ttf -s 12 -l ${PROJECT_BINARY_DIR}/charis_comparison.log -c -g -n)
    add_test(NAME padauk_comparison COMMAND $<TARGET_FILE:comparerenderer>
        -t ${PROJECT_SOURCE_DIR}/../texts/my_HeadwordSyllables.txt -f ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf -s 12 -l ${PROJECT_BINARY_DIR}/padauk_comparison.log -c -g -n)

endif (${GR_FOUND})

# comparerenderer can also be used for font testing
install(TARGETS comparerenderer EXPORT comparerenderer RUNTIME DESTINATION bin)
