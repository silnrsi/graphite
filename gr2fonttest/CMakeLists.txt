CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

project(gr2fonttest)

enable_testing()

include_directories(${PROJECT_SOURCE_DIR})

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	# Checkout from http://win-iconv.googlecode.com/svn/trunk
	find_path(WIN_ICONV_PATH win_iconv.c PATHS ../../win-iconv 
		DOC "Checkout from http://win-iconv.googlecode.com/svn/trunk")
	if (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
		add_subdirectory(${WIN_ICONV_PATH} win-iconv)
		include_directories(${WIN_ICONV_PATH})
		add_definitions(-DHAVE_ICONV)
	endif (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
	add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DUNICODE)
else (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	find_path(ICONV_HEADER_PATH iconv.h)
	if (NOT ${ICONV_HEADER_PATH} STREQUAL "ICONV_HEADER_PATH-NOTFOUND")
		include_directories(${ICONV_HEADER_PATH})
		add_definitions(-DHAVE_ICONV)
	endif (NOT ${ICONV_HEADER_PATH} STREQUAL "ICONV_HEADER_PATH-NOTFOUND")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

if (NOT DISABLE_TRACING)
	if (ENABLE_DEEP_TRACING)
		add_definitions(-DENABLE_DEEP_TRACING)
	endif (ENABLE_DEEP_TRACING)
endif (NOT DISABLE_TRACING)

add_executable(gr2fonttest gr2FontTest.cpp)
target_link_libraries(gr2fonttest graphite2)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
        target_link_libraries(gr2fonttest iconv)
        add_dependencies(gr2fonttest iconv)
    endif (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # -lgcc LINKER_LANGUAGE C
    add_definitions(-fno-rtti -fno-exceptions)
    set_target_properties(gr2fonttest PROPERTIES LINK_FLAGS "-nodefaultlibs" )
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
    # This script just fails
    nolib_test(stdc++ $<TARGET_FILE:gr2fonttest>)
endif  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

# copy the DLL so that gr2fonttest can find it
add_custom_target(${PROJECT_NAME}_copy_dll ALL 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphite2_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphite2${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
        add_custom_command(TARGET ${PROJECT_NAME}_copy_dll POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${win_iconv_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}iconv${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    endif (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_dependencies(${PROJECT_NAME}_copy_dll graphite2 iconv gr2fonttest)

install(TARGETS gr2fonttest EXPORT gr2fonttest RUNTIME DESTINATION bin)

