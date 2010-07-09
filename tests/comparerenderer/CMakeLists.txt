project(comparerenderer)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    #find_package(Freetype)
    find_package(PkgConfig)

    pkg_check_modules(GR silgraphite)
    pkg_check_modules(HB harfbuzz)
    # pkg_check_modules(GRNG graphite)

    if (${HB_FOUND})
        add_definitions(-DHAVE_HARFBUZZ_HB_COMMON_H)
    endif (${HB_FOUND})

    if (${GR_FOUND})
        add_definitions(-DHAVE_GRAPHITE)
    endif (${GR_FOUND})

endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include_directories(../common icule ${FREETYPE_INCLUDE_DIRS} ${HB_INCLUDE_DIRS} ${GR_INCLUDE_DIRS})

add_executable(comparerenderer
    CompareRenderer.cpp
    GrUtfTextSrc.cpp
    )

# rt is for timing
target_link_libraries(comparerenderer graphiteng ${HB_LDFLAGS} ${GR_LDFLAGS} rt)
