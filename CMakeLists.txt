CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0012 NEW)
include(CMakeDependentOption)
include(TestBigEndian)

project(graphite2)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if (NOT CMAKE_BUILD_TYPE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE Release)
endif (NOT CMAKE_BUILD_TYPE)

enable_language(CXX C)

include_directories(${PROJECT_SOURCE_DIR}/include)

enable_testing()

option(DISABLE_TRACING "Disable code required for using the XML Trace logging" ON)
option(DISABLE_SEGCACHE "Don't build the segment cache support code" OFF)
option(DISABLE_FILEFACE "Don't build with fileface support" OFF)
cmake_dependent_option(ENABLE_DEEP_TRACING "Enables time consuming deep Trace logging" OFF
                        "NOT DISABLE_TRACING" OFF)
set(VM_MACHINE_TYPE auto CACHE STRING "Choose the type of vm machine: Auto, Direct or Call.")
set(DOXYGEN_CONFIG public CACHE STRING "Specifies the base configuration file for doxygen. Current values: public, all")


message(STATUS "Build is " ${CMAKE_BUILD_TYPE})
string(REPLACE "ON" "off" _tracing ${DISABLE_TRACING})
string(REPLACE "OFF" "on" _tracing ${_tracing})
message(STATUS "Tracing is " ${_tracing})
string(TOLOWER ${ENABLE_DEEP_TRACING} _deep_tracing)
message(STATUS "Deep tracing is " ${_deep_tracing})

if (DISABLE_TRACING)
    add_definitions(-DDISABLE_TRACING)
endif (DISABLE_TRACING)


string(TOLOWER ${VM_MACHINE_TYPE} VM_MACHINE_TYPE)
if (NOT VM_MACHINE_TYPE MATCHES "auto|direct|call")
    message(SEND_ERROR "unrecognised vm machine type: ${VM_MACHINE_TYPE}. Only Auto, Direct or Call are available")
endif (NOT VM_MACHINE_TYPE MATCHES "auto|direct|call")
if (VM_MACHINE_TYPE STREQUAL "auto")
    if (CMAKE_BUILD_TYPE MATCHES "[Rr]el(ease|[Ww]ith[Dd]eb[Ii]nfo)")
        set(VM_MACHINE_TYPE "direct")
    else(CMAKE_BUILD_TYPE MATCHES "[Rr]el(ease|[Ww]ith[Dd]eb[Ii]nfo)")
        set(VM_MACHINE_TYPE "call")
    endif(CMAKE_BUILD_TYPE MATCHES "[Rr]el(ease|[Ww]ith[Dd]eb[Ii]nfo)")
endif (VM_MACHINE_TYPE STREQUAL "auto")
if (VM_MACHINE_TYPE STREQUAL "direct" AND NOT CMAKE_COMPILER_IS_GNUCXX)
    message(WARNING "vm machine type direct can only be built using GCC")
    set(VM_MACHINE_TYPE "call")
endif (VM_MACHINE_TYPE STREQUAL "direct" AND NOT CMAKE_COMPILER_IS_GNUCXX)
message(STATUS "Using vm machine type: ${VM_MACHINE_TYPE}")

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
if (WORDS_BIGENDIAN)
    add_definitions(-DWORDS_BIGENDIAN)
endif (WORDS_BIGENDIAN)


add_subdirectory(src)
add_subdirectory(gr2fonttest)
add_subdirectory(tests)
add_subdirectory(doc)
set(version 2.0.0)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
set(includedir ${CMAKE_INSTALL_PREFIX}/include)

configure_file(graphite2.pc.in graphite2.pc)

install(FILES ${PROJECT_BINARY_DIR}/graphite2.pc DESTINATION lib${LIB_SUFFIX}/pkgconfig)

