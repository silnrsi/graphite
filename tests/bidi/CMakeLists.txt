project(biditesting)

add_custom_target(biditest)
add_custom_command(TARGET biditest COMMAND python ${PROJECT_SOURCE_DIR}/biditest ARGS -c ${PROJECT_SOURCE_DIR}/BidiCharacterTest.txt -t ${PROJECT_SOURCE_DIR}/BidiTest.txt ${PROJECT_SOURCE_DIR}/../fonts/Scheherazadegr.ttf)

