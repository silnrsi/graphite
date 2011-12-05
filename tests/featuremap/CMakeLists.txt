project(featuremaptest)
include(Graphite)
include_directories(${graphite2_core_SOURCE_DIR})

add_definitions(-DGR2_EXPORTING)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
    add_custom_target(${PROJECT_NAME}_copy_dll ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    add_dependencies(${PROJECT_NAME}_copy_dll graphite2 featuremaptest)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

set(S ${graphite2_core_SOURCE_DIR})
add_executable(featuremaptest featuremaptest.cpp)
target_link_libraries(featuremaptest graphite2 graphite2-base)

add_test(NAME featuremaptest COMMAND $<TARGET_FILE:featuremaptest> ${testing_SOURCE_DIR}/fonts/tiny.ttf)
