CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

project(grngfonttest)

enable_testing()

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../common ${PROJECT_SOURCE_DIR}/../src)
add_executable(grngfonttest grngFontTest.cpp)

target_link_libraries(grngfonttest graphiteng)

find_file(PADAUK_FONT Padauk.ttf PATHS /usr/share/fonts/truetype/ttf-sil-padauk $ENV{HOME}/.fonts $ENV{SystemRoot}/Fonts)
find_file(CHARIS_FONT CharisSILR.ttf PATHS /usr/share/fonts/truetype/ttf-sil-charis $ENV{HOME}/.fonts $ENV{SystemRoot}/Fonts)

# TODO need to set ENVIRONMENT property for Windows?
add_test(NAME grngfonttestPadauk1 COMMAND $<TARGET_FILE:grngfonttest> ${PADAUK_FONT}  -log ${PROJECT_BINARY_DIR}/grngfonttestPadauk1.log -codes 1000 1031 102C)
# TODO Add expected results to compare with
#add_test(NAME grngfonttestPadauk1Output COMMAND ${CMAKE_COMMAND} -E compare_files ${PROJECT_BINARY_DIR}/grngfonttestPadauk1.log ${PROJECT_SOURCE_DIR}/expected/grngfonttestPadauk1.log)
#set_tests_properties(grngfonttestPadauk1Output PROPERTIES DEPENDS grngfonttestPadauk1)

add_test(NAME grngfonttestCharis1 COMMAND $<TARGET_FILE:grngfonttest> ${CHARIS_FONT}  -log ${PROJECT_BINARY_DIR}/grngfonttestPadauk1.log -codes 0069 02e6 02e8 02e5)

