
project(featuremaptest)

include_directories(../../src ${PROJECT_BINARY_DIR}/../../src)
INCLUDE(CheckTypeSize)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T} -DGR2_EXPORTING)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_executable(featuremaptest featuremaptest.cpp
    ../../src/FeatureMap.cpp
    ../../src/XmlTraceLog.cpp
    ../../src/XmlTraceLogTags.cpp)

add_test(NAME featuremaptest COMMAND $<TARGET_FILE:featuremaptest>)
