CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

project(graphiteng)
enable_language(CXX C)

include_directories(${PROJECT_SOURCE_DIR}/include)

enable_testing()

add_subdirectory(src)
add_subdirectory(grngfonttest)
set(version 3.0.0)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(includedir ${CMAKE_INSTALL_PREFIX}/include)

configure_file(graphite.pc.in graphite.pc)

install(FILES ${PROJECT_BINARY_DIR}/graphite.pc DESTINATION lib/pkgconfig)

