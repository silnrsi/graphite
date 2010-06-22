CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
project(graphiteng-core)
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

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../common )

set(GRAPHITE_HEADERS 
    ../include/graphiteng/IFont.h
    ../include/graphiteng/IFace.h
    ../include/graphiteng/IFeatures.h
    ../include/graphiteng/ISegment.h
    ../include/graphiteng/ISlot.h
    ../include/graphiteng/ITextSource.h
    ../include/graphiteng/Types.h
    )

add_library(graphiteng SHARED
    create.cpp
    FeatureMap.cpp
    FontFace.cpp
    FontImpl.cpp
    Segment.cpp
    Slot.cpp
    Silf.cpp
    Pass.cpp
    TtfUtil.cpp
    XmlTraceLog.cpp
    XmlTraceLogTags.cpp
    ../common/FileFont.h
    ../common/FileFont.cpp
	${GRAPHITE_HEADERS}
    )

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-Wno-unknown-pragmas -Wparentheses -Wextra -Wendif-labels
     -Wshadow -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor
     -fno-rtti -fno-exceptions -nodefaultlibs -nostdlibs)
     set_target_properties(graphiteng PROPERTIES LINK_FLAGS "-nostdlibs -lgcc -lm" LINKER_LANGUAGE C)
     set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
     add_test(NAME nostdc++ COMMAND ldd $<TARGET_SONAME_FILE:graphiteng>)
     set_tests_properties(nostdc++ PROPERTIES FAIL_REGULAR_EXPRESSION "libstdc\\+\\+")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


set_target_properties(graphiteng PROPERTIES PUBLIC_HEADER "${GRAPHITE_HEADERS}")

set_target_properties(graphiteng PROPERTIES SOVERSION ${GRAPHITE_SO_VERSION})
install(TARGETS graphiteng EXPORT graphiteng LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include/graphiteng)
install(EXPORT graphiteng DESTINATION share/graphiteng NAMESPACE gr_)


