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
