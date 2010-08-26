project(docs)

find_program(A2X a2x)

if(A2X)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/manual.pdf
                       DEPENDS ${PROJECT_SOURCE_DIR}/*.txt
                       COMMAND ${A2X} --icons -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/manual.txt)
    add_custom_target(docs ALL DEPENDS ${PROJECT_BINARY_DIR}/manual.pdf)
endif(A2X)

