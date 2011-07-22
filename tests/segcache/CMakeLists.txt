CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
project(grsegcachetest)

include_directories(../../src)
INCLUDE(CheckTypeSize)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T} -DGR2_EXPORTING)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_executable(grsegcachetest segcachetest.cpp ../../src/TtfUtil.cpp
    ../../src/Bidi.cpp
    ../../src/call_machine.cpp
    ../../src/CachedFace.cpp
    ../../src/CmapCache.cpp
    ../../src/Code.cpp
    ../../src/Face.cpp
    ../../src/FeatureMap.cpp
    ../../src/Font.cpp
    ../../src/GlyphFace.cpp
    ../../src/GlyphFaceCache.cpp
    ../../src/gr_face.cpp
    ../../src/gr_features.cpp
    ../../src/gr_font.cpp
    ../../src/gr_logging.cpp
    ../../src/gr_segment.cpp
    ../../src/NameTable.cpp
    ../../src/Pass.cpp
    ../../src/SegCache.cpp
    ../../src/SegCacheEntry.cpp
    ../../src/SegCacheStore.cpp
    ../../src/Segment.cpp
    ../../src/Silf.cpp
    ../../src/Slot.cpp
    ../../src/XmlTraceLog.cpp
    ../../src/XmlTraceLogTags.cpp)


add_test(NAME grsegcachetest COMMAND $<TARGET_FILE:grsegcachetest> ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf)
