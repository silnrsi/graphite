CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

project(grngfonttest)

enable_testing()

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../common ${PROJECT_SOURCE_DIR}/../src)

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

add_executable(grngfonttest grngFontTest.cpp)

target_link_libraries(grngfonttest graphiteng)
add_dependencies(grngfonttest graphiteng)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
        target_link_libraries(grngfonttest iconv)
        add_dependencies(grngfonttest iconv)
    endif (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

# find_file(PADAUK_FONT Padauk.ttf PATHS /usr/share/fonts/truetype/ttf-sil-padauk $ENV{HOME}/.fonts $ENV{SystemRoot}/Fonts)
# find_file(CHARIS_FONT CharisSILR.ttf PATHS /usr/share/fonts/truetype/ttf-sil-charis $ENV{HOME}/.fonts $ENV{SystemRoot}/Fonts)

if  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # -lgcc LINKER_LANGUAGE C
    add_definitions(-fno-rtti -fno-exceptions)
    set_target_properties(grngfonttest PROPERTIES LINK_FLAGS "-nostdlibs -nodefaultlibs -lc" )
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
    # This script just fails
    add_test(NAME grngfonttest-nostdc++ COMMAND ldd $<TARGET_SONAME_FILE:graphiteng>)
    set_tests_properties(grngfonttest-nostdc++ PROPERTIES FAIL_REGULAR_EXPRESSION "libstdc\\+\\+")

endif  (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

# copy the DLL so that grngfonttest can find it
add_custom_target(copy_dll ALL 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${graphiteng_core_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphiteng${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
        add_custom_command(TARGET copy_dll POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${win_iconv_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}iconv${CMAKE_SHARED_LIBRARY_SUFFIX} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    endif (NOT ${WIN_ICONV_PATH} STREQUAL "WIN_ICONV_PATH-NOTFOUND")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

add_dependencies(copy_dll graphiteng iconv)

# TODO need to set ENVIRONMENT property for Windows?
# add_test(NAME grngfonttestPadauk1 COMMAND $<TARGET_FILE:grngfonttest> ${PADAUK_FONT}  -log ${PROJECT_BINARY_DIR}/grngfonttestPadauk1.log -trace ${PROJECT_BINARY_DIR}/grngfonttestPadauk1.xml -codes 1000 1031 102C)
# TODO Add expected results to compare with
#add_test(NAME grngfonttestPadauk1Output COMMAND ${CMAKE_COMMAND} -E compare_files ${PROJECT_BINARY_DIR}/grngfonttestPadauk1.log ${PROJECT_SOURCE_DIR}/expected/grngfonttestPadauk1.log)
#set_tests_properties(grngfonttestPadauk1Output PROPERTIES DEPENDS grngfonttestPadauk1)

# add_test(NAME grngfonttestCharis1 COMMAND $<TARGET_FILE:grngfonttest> ${CHARIS_FONT}  -log ${PROJECT_BINARY_DIR}/grngfonttestCharis1.log -trace ${PROJECT_BINARY_DIR}/grngfonttestCharis1.xml -codes 0069 02e6 02e8 02e5)

# set_tests_properties(grngfonttestPadauk1 grngfonttestCharis1 PROPERTIES ENVIRONMENT LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
