project(docs)

set(GRAPHITE2_DOXYGEN_CONFIG public CACHE STRING "Specifies the base configuration file for doxygen. Current values: public, all")

find_program(A2X a2x)
find_program(DBLATEX dblatex)
find_program(DOXYGEN doxygen)
find_program(LATEX pdflatex)

set(DOC_DEPENDS)
if(A2X)
    if(DBLATEX)
        add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/manual.pdf
                           DEPENDS ${PROJECT_SOURCE_DIR}/[a-z]*.txt
                           COMMAND ${A2X} --icons -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/manual.txt)
        add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/GTF.pdf
                           DEPENDS ${PROJECT_SOURCE_DIR}/GTF.txt
                           COMMAND ${A2X} --icons -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/GTF.txt)
        set(DOC_DEPENDS ${PROJECT_BINARY_DIR}/manual.pdf ${PROJECT_BINARY_DIR}/GTF.pdf)
    endif(DBLATEX)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/manual.html
                       DEPENDS ${PROJECT_SOURCE_DIR}/[a-z]*.txt ${PROJECT_SOURCE_DIR}/graphite.css
                       COMMAND ${A2X} -f xhtml --stylesheet=graphite.css -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/manual.txt)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/GTF.html
                       DEPENDS ${PROJECT_SOURCE_DIR}/GTF.txt ${PROJECT_SOURCE_DIR}/graphite.css
                       COMMAND ${A2X} -f xhtml --stylesheet=graphite.css -D ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/GTF.txt)
    set(DOC_DEPENDS ${DOC_DEPENDS} ${PROJECT_BINARY_DIR}/manual.html ${PROJECT_BINARY_DIR}/GTF.html)
endif(A2X)

if(DOXYGEN AND LATEX)
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/doxygen/latex/refman.pdf
                        DEPENDS ${PROJECT_SOURCE_DIR}/../include/graphite2/*.h
                        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/${GRAPHITE2_DOXYGEN_CONFIG}.cfg
                        COMMAND ${DOXYGEN} ${PROJECT_SOURCE_DIR}/${GRAPHITE2_DOXYGEN_CONFIG}.cfg
                        COMMAND make -C ${PROJECT_BINARY_DIR}/doxygen/latex)
    set(DOC_DEPENDS ${DOC_DEPENDS} ${PROJECT_BINARY_DIR}/doxygen/latex/refman.pdf)
endif(DOXYGEN AND LATEX)

add_custom_target(docs DEPENDS ${DOC_DEPENDS})
