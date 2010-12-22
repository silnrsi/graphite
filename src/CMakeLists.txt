#    GRAPHITENG LICENSING
#
#    Copyright 2010, SIL International
#    All rights reserved.
#
#    This library is free software; you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published
#    by the Free Software Foundation; either version 2.1 of License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should also have received a copy of the GNU Lesser General Public
#    License along with this library in the file named "LICENSE".
#    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
#    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
#    internet at http://www.fsf.org/licenses/lgpl.html.

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
project(graphite2_core)
cmake_policy(SET CMP0012 NEW)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCXXSourceCompiles)

set(GRAPHITE_API_MAJOR 0)
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
    set(TRACE_HEADERS ../include/graphite2/XmlLog.h)
    if (ENABLE_DEEP_TRACING)
        add_definitions(-DENABLE_DEEP_TRACING)
    endif (ENABLE_DEEP_TRACING)
endif (NOT DISABLE_TRACING)

set(GRAPHITE_HEADERS 
    ../include/graphite2/Font.h
    ../include/graphite2/Segment.h
    ../include/graphite2/Types.h
    ${TRACE_HEADERS}
    )

file(GLOB PRIVATE_HEADERS *.h) 

add_library(graphite2 SHARED
    ${VM_MACHINE_TYPE}_machine.cpp
    gr_char_info.cpp
    gr_feature_ref.cpp
    gr_features.cpp
    gr_face.cpp
    gr_font.cpp
    gr_logging.cpp
    gr_segment.cpp
    gr_slot.cpp
    CachedFace.cpp
    CmapCache.cpp
    Code.cpp
    Face.cpp
    FeatureMap.cpp
    Font.cpp
    GlyphFace.cpp
    GlyphFaceCache.cpp
    Main.cpp
    NameTable.cpp
    Pass.cpp
    SegCache.cpp
    SegCacheEntry.cpp
    SegCacheStore.cpp
    Segment.cpp
    Silf.cpp
    Slot.cpp
    TtfUtil.cpp
    XmlTraceLog.cpp
    XmlTraceLogTags.cpp
    ${GRAPHITE_HEADERS}
    ${PRIVATE_HEADERS}
    )

set_source_files_properties(${PRIVATE_HEADERS} PROPERTIES HEADER_FILE_ONLY true)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-Wall -Wno-unknown-pragmas -Wparentheses -Wextra -Wendif-labels
     -Wshadow -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor -fdiagnostics-show-option
     -fno-rtti -fno-exceptions -nodefaultlibs -nostdlibs -fvisibility=hidden -fno-stack-protector)
    set_target_properties(graphite2 PROPERTIES LINK_FLAGS "-nostdlibs -nodefaultlibs" LINKER_LANGUAGE C)
    target_link_libraries(graphite2 "-lc -lgcc")
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
    nolib_test(stdc++ $<TARGET_SONAME_FILE:graphite2>)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


set_target_properties(graphite2 PROPERTIES PUBLIC_HEADER "${GRAPHITE_HEADERS}")

set_target_properties(graphite2 PROPERTIES SOVERSION ${GRAPHITE_SO_VERSION})
install(TARGETS graphite2 EXPORT graphite2 LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include/graphite2)
install(EXPORT graphite2 DESTINATION share/graphite2 NAMESPACE gr2_)

