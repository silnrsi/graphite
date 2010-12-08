CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0012 NEW)
project(graphite2)
enable_language(CXX C)

include_directories(${PROJECT_SOURCE_DIR}/include)

enable_testing()

function(nolib_test LIBNAME OBJECTFILE)
    string(REGEX REPLACE "[][^$.*+?|()-]" "\\\\\\0" LIBNAME_REGEX ${LIBNAME})
    add_test(NAME nolib-${LIBNAME}-${PROJECT_NAME}
            COMMAND readelf --dynamic ${OBJECTFILE})
    set_tests_properties(nolib-${LIBNAME}-${PROJECT_NAME} PROPERTIES 
            FAIL_REGULAR_EXPRESSION "0x[0-9a-f]+ \\(NEEDED\\)[ \\t]+Shared library: \\[${CMAKE_SHARED_LIBRARY_PREFIX}${LIBNAME_REGEX}${CMAKE_SHARED_LIBRARY_SUFFIX}.*\\]")
endfunction(nolib_test)

option(DISABLE_TRACING "Disable code required for using the XML Trace logging")

option(ENABLE_DEEP_TRACING "Enables time consuming deep Trace logging")

set(VM_MACHINE_TYPE call CACHE STRING "Choose the type of vm machine: Auto, Direct or Call.   Auto -- depends upon configuration type; Direct -- usually fastest but only on GCC. default for Release configuration with GCC; Call -- slower but universal. defualt for Debug configuration")

set(DOXYGEN_CONFIG public CACHE STRING "Specifies the base configuration file for doxygen. Current values: public, all")

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

if (DISABLE_TRACING)
    add_definitions(-DDISABLE_TRACING)
    message(STATUS "Tracing disabled")
	if (ENABLE_DEEP_TRACING)
		message(STATUS "Deep tracing request ignored!")
	endif (ENABLE_DEEP_TRACING)
else(DISABLE_TRACING)
	if (ENABLE_DEEP_TRACING)
		message(STATUS "Deep tracing enabled")
		# The actual define is now set only in the projects which want it
	endif (ENABLE_DEEP_TRACING)
endif (DISABLE_TRACING)

add_subdirectory(src)
add_subdirectory(grngfonttest)
add_subdirectory(tests)
add_subdirectory(doc)
set(version 0.0.0)
set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
set(includedir ${CMAKE_INSTALL_PREFIX}/include)

configure_file(graphite2.pc.in graphite2.pc)

install(FILES ${PROJECT_BINARY_DIR}/graphite.pc DESTINATION lib/pkgconfig)

