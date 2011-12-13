CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0012 NEW)
include(TestBigEndian)

project(graphite2)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if (NOT CMAKE_BUILD_TYPE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE Release)
endif (NOT CMAKE_BUILD_TYPE)

enable_language(CXX C)

include_directories(${PROJECT_SOURCE_DIR}/include)

enable_testing()

set(VM_MACHINE_TYPE auto CACHE STRING "Choose the type of vm machine: Auto, Direct or Call.")
set(DOXYGEN_CONFIG public CACHE STRING "Specifies the base configuration file for doxygen. Current values: public, all")


message(STATUS "Build is " ${CMAKE_BUILD_TYPE})


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


add_subdirectory(src)
add_subdirectory(gr2fonttest)
add_subdirectory(tests)
add_subdirectory(doc)
set(version 2.0.0)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
set(includedir ${CMAKE_INSTALL_PREFIX}/include)

configure_file(graphite2.pc.in graphite2.pc)

install(FILES ${PROJECT_BINARY_DIR}/graphite2.pc DESTINATION lib${LIB_SUFFIX}/pkgconfig)

