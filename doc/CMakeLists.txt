project(docs)

find_program(A2X a2x)
find_program(DBLATEX dblatex)

if(A2X)
    if(DBLATEX)
        add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/manual.pdf
                           DEPENDS ${PROJECT_SOURCE_DIR}/*.txt
                           COMMAND ${A2X} --icons -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/manual.txt)
        add_custom_target(docs ALL DEPENDS ${PROJECT_BINARY_DIR}/manual.pdf)
    endif(DBLATEX)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/manual.html
                       DEPENDS ${PROJECT_SOURCE_DIR}/*.txt
                       COMMAND ${A2X} -f xhtml --icons -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/manual.txt)
    add_custom_target(docs ALL DEPENDS ${PROJECT_BINARY_DIR}/manual.html)
endif(A2X)

