project(docs)

find_program(A2X a2x)
find_program(DBLATEX dblatex)
find_program(DOXYGEN doxygen)
find_program(LATEX xelatex)

if(A2X)
    set(DOC_DEPENDS)
    if(DBLATEX)
        add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/manual.pdf
                           DEPENDS ${PROJECT_SOURCE_DIR}/*.txt
                           COMMAND ${A2X} --icons -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/manual.txt)
        set(DOC_DEPENDS ${PROJECT_BINARY_DIR}/manual.pdf)
    endif(DBLATEX)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/manual.html
                       DEPENDS ${PROJECT_SOURCE_DIR}/*.txt
                       COMMAND ${A2X} -f xhtml --icons -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/manual.txt)
    add_custom_target(docs ALL DEPENDS ${DOC_DEPENDS} ${PROJECT_BINARY_DIR}/manual.html)
endif(A2X)

if(DOXYGEN AND LATEX)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/doxygen/latex/refman.pdf
                        DEPENDS ${PROJECT_SOURCE_DIR}/../include/graphiteng/*.h
                        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/${DOXYGEN_CONFIG}.cfg
                        COMMAND ${DOXYGEN} ${PROJECT_SOURCE_DIR}/${DOXYGEN_CONFIG}.cfg
                        COMMAND make -C ${PROJECT_BINARY_DIR}/doxygen/latex)
    set(REFMAN_DEPENDS ${PROJECT_BINARY_DIR}/doxygen/latex/refman.pdf)
    add_custom_target(refman ALL DEPENDS ${REFMAN_DEPENDS} ${PROJECT_BINARY_DIR}/doxygen/latex/refman.pdf)
endif(DOXYGEN AND LATEX)
