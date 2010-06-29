CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0012 NEW)
project(graphiteng)
enable_language(CXX C)

include_directories(${PROJECT_SOURCE_DIR}/include)

enable_testing()

option(DISABLE_TRACING "Disable code required for using the XML Trace logging")

if (DISABLE_TRACING)
    add_definitions(-DDISABLE_TRACING)
    message(STATUS "Tracing disabled")
endif (DISABLE_TRACING)

option(ENABLE_DEEP_TRACING "Enables time consuming deep Trace logging")

if (ENABLE_DEEP_TRACING)
    add_definitions(-DENABLE_DEEP_TRACING)
    message(STATUS "Deep tracing enabled")
endif (ENABLE_DEEP_TRACING)

add_subdirectory(src)
add_subdirectory(grngfonttest)
add_subdirectory(tests)
set(version 3.0.0)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(includedir ${CMAKE_INSTALL_PREFIX}/include)

configure_file(graphite.pc.in graphite.pc)

install(FILES ${PROJECT_BINARY_DIR}/graphite.pc DESTINATION lib/pkgconfig)

