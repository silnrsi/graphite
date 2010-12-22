CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
project(grsegcachetest)

include_directories(../../src)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_executable(grsegcachetest segcachetest.cpp ../../src/TtfUtil.cpp
    ../../src/call_machine.cpp
    ../../src/CmapCache.cpp
    ../../src/Code.cpp
    ../../src/FeatureMap.cpp
    ../../src/GlyphFace.cpp
    ../../src/GlyphFaceCache.cpp
    ../../src/gr_face.cpp
    ../../src/gr_features.cpp
    ../../src/gr_font.cpp
    ../../src/gr_logging.cpp
    ../../src/gr_segment.cpp
    ../../src/GrCachedFace.cpp
    ../../src/GrFaceImp.cpp
    ../../src/GrFontImp.cpp
    ../../src/GrSegmentImp.cpp
    ../../src/NameTable.cpp
    ../../src/Pass.cpp
    ../../src/Silf.cpp
    ../../src/SlotImp.cpp
    ../../src/SegCache.cpp
    ../../src/SegCacheEntry.cpp
    ../../src/SegCacheStore.cpp
    ../../src/XmlTraceLog.cpp
    ../../src/XmlTraceLogTags.cpp)


add_test(NAME grsegcachetest COMMAND $<TARGET_FILE:grsegcachetest> ${PROJECT_SOURCE_DIR}/../fonts/Padauk.ttf)
