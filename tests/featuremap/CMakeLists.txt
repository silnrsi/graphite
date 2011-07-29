
project(featuremaptest)

set(SRC ${PROJECT_SOURCE_DIR}/../../src)
include_directories(${PROJECT_SOURCE_DIR} ${SRC})

INCLUDE(CheckTypeSize)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T} -DGR2_EXPORTING)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_executable(featuremaptest featuremaptest.cpp
				${SRC}/Bidi.cpp
				${SRC}/call_machine.cpp
                ${SRC}/Code.cpp
                ${SRC}/CmapCache.cpp
                ${SRC}/Face.cpp
                ${SRC}/FeatureMap.cpp
                ${SRC}/GlyphFace.cpp
                ${SRC}/GlyphFaceCache.cpp
                ${SRC}/NameTable.cpp
                ${SRC}/Pass.cpp
                ${SRC}/Segment.cpp
                ${SRC}/Silf.cpp
                ${SRC}/Slot.cpp
                ${SRC}/Sparse.cpp
                ${SRC}/TtfUtil.cpp
     			${SRC}/XmlTraceLog.cpp
    			${SRC}/XmlTraceLogTags.cpp)

add_test(NAME featuremaptest COMMAND $<TARGET_FILE:featuremaptest>)
