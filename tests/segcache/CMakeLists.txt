project(grsegcachetest)

include_directories(../../src)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})

add_executable(grsegcachetest segcachetest.cpp ../../src/TtfUtil.cpp
    ../../src/call_machine.cpp
    ../../src/CmapCache.cpp
    ../../src/Code.cpp
    ../../src/FeatureMap.cpp
    ../../src/Features.cpp
    ../../src/GlyphFace.cpp
    ../../src/GlyphFaceCache.cpp
    ../../src/GrCachedFace.cpp
    ../../src/GrFace.cpp
    ../../src/GrFaceImp.cpp
    ../../src/GrFont.cpp
    ../../src/GrFontImp.cpp
    ../../src/GrSegment.cpp
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
