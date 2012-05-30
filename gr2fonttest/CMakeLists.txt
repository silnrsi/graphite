CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

project(gr2fonttest)

enable_testing()

include_directories(${PROJECT_SOURCE_DIR})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_executable(gr2fonttest gr2FontTest.cpp UtfCodec.cpp)
target_link_libraries(gr2fonttest graphite2)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # -lgcc LINKER_LANGUAGE C
    add_definitions(-fno-rtti -fno-exceptions)
    set_target_properties(gr2fonttest PROPERTIES LINK_FLAGS "-nodefaultlibs" LINKER_LANGUAGE C)
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
    # This script just fails
    nolib_test(stdc++ $<TARGET_FILE:gr2fonttest>)
endif  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

# copy the DLL so that gr2fonttest can find it
add_custom_target(${PROJECT_NAME}_copy_dll ALL 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

add_dependencies(${PROJECT_NAME}_copy_dll graphite2 gr2fonttest)

install(TARGETS gr2fonttest EXPORT gr2fonttest RUNTIME DESTINATION bin)

