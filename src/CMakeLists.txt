CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
project(graphiteng_core)
cmake_policy(SET CMP0012 NEW)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCXXSourceCompiles)

set(GRAPHITE_API_MAJOR 3)
set(GRAPHITE_API_MINOR 0)
set(GRAPHITE_API_PATCH 0)
set(GRAPHITE_SO_VERSION ${GRAPHITE_API_MAJOR}.${GRAPHITE_API_MINOR}.${GRAPHITE_API_PATCH})

# check for fabsf definition
set(FABSF_TEST_SRC "#include <cmath>\nint main() { fabsf(1.2); }")
CHECK_CXX_SOURCE_COMPILES("${FABSF_TEST_SRC}" HAVE_FABSF)
if (HAVE_FABSF)
add_definitions(-DHAVE_FABSF)
endif (HAVE_FABSF)

#check sizeof wchar_t
CHECK_TYPE_SIZE(wchar_t SIZEOF_WCHAR_T)
add_definitions(-DSIZEOF_WCHAR_T=${SIZEOF_WCHAR_T})
CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})


include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../common )

if (NOT DISABLE_TRACING)
    message(STATUS "Tracing enabled")
    set(TRACE_HEADERS ../include/graphiteng/XmlLog.h)
endif (NOT DISABLE_TRACING)

set(GRAPHITE_HEADERS 
    ../include/graphiteng/AutoHandle.h
    ../include/graphiteng/FeatureRefHandle.h
    ../include/graphiteng/FeaturesHandle.h
    ../include/graphiteng/IFont.h
    ../include/graphiteng/IFace.h
    ../include/graphiteng/RefCountHandle.h
    ../include/graphiteng/SegmentHandle.h
    ../include/graphiteng/SlotHandle.h
    ../include/graphiteng/Types.h
    ${TRACE_HEADERS}
    )

file(GLOB PRIVATE_HEADERS *.h) 

add_library(graphiteng SHARED
    call_machine.cpp
    Code.cpp
    FeatureMap.cpp
    FeatureRefHandle.cpp
    FeaturesHandle.cpp
    GlyphFace.cpp
    LoadedFace.cpp
    LoadedFont.cpp
    IFace.cpp
    IFont.cpp
    Segment.cpp
    SegmentHandle.cpp
    Slot.cpp
    SlotHandle.cpp
    Silf.cpp
    Pass.cpp
    TtfUtil.cpp
    XmlTraceLog.cpp
    XmlTraceLogTags.cpp
	${GRAPHITE_HEADERS}
	${PRIVATE_HEADERS}
    )

set_source_files_properties(${PRIVATE_HEADERS} PROPERTIES HEADER_FILE_ONLY true)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-Wno-unknown-pragmas -Wparentheses -Wextra -Wendif-labels
     -Wshadow -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor
     -fno-rtti -fno-exceptions -nodefaultlibs -nostdlibs -fvisibility=hidden)
     set_target_properties(graphiteng PROPERTIES LINK_FLAGS "-nostdlibs -lgcc -lm" LINKER_LANGUAGE C)
     set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
#     add_test(NAME nostdc++ COMMAND ldd $<TARGET_SONAME_FILE:graphiteng>)
#     set_tests_properties(nostdc++ PROPERTIES FAIL_REGULAR_EXPRESSION "libstdc\\+\\+")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


set_target_properties(graphiteng PROPERTIES PUBLIC_HEADER "${GRAPHITE_HEADERS}")

set_target_properties(graphiteng PROPERTIES SOVERSION ${GRAPHITE_SO_VERSION})
install(TARGETS graphiteng EXPORT graphiteng LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include/graphiteng)
install(EXPORT graphiteng DESTINATION share/graphiteng NAMESPACE gr_)


