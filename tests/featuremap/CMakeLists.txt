project(featuremaptest)
include(Graphite)
include_directories(../../src)

add_definitions(-DGR2_EXPORTING)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

target_objects(OBJS graphite2)
add_executable(featuremaptest featuremaptest.cpp ${OBJS})
add_dependencies(featuremaptest graphite2)

add_test(NAME featuremaptest COMMAND $<TARGET_FILE:featuremaptest>)
