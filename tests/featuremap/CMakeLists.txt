
project(featuremaptest)

include_directories(../../src)

CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
add_definitions(-DSIZEOF_SIZE_T=${SIZEOF_SIZE_T})

add_executable(featuremaptest featuremaptest.cpp
    ../../src/FeatureMap.cpp
    ../../src/Features.cpp
    ../../src/XmlTraceLog.cpp
    ../../src/XmlTraceLogTags.cpp)

add_test(NAME featuremaptest COMMAND $<TARGET_FILE:featuremaptest>)
