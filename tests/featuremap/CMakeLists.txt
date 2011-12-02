project(featuremaptest)
include(Graphite)
include_directories(${graphite2_core_SOURCE_DIR})

add_definitions(-DGR2_EXPORTING)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

set(S ${graphite2_core_SOURCE_DIR})
add_executable(featuremaptest featuremaptest.cpp)
target_link_libraries(featuremaptest graphite2 graphite2-base)

add_test(NAME featuremaptest COMMAND $<TARGET_FILE:featuremaptest> ${PROJECT_SOURCE_DIR}/../fonts/tiny.ttf)
