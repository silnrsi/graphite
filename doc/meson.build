a2x      = find_program('a2x',      required: false)
dblatex  = find_program('dblatex',  required: false)
doxygen  = find_program('doxygen',  required: false)
pdflatex = find_program('pdflatex', required: false)

installdir = join_paths(get_option('datadir'), 'doc', meson.project_name())

if a2x.found()
    a2x_html = [a2x,
        '--format=xhtml',
        '--stylesheet=graphite.css',
        '--destination-dir=@OUTDIR@', '@INPUT@'
    ]
    manual_deps = files(
      'building.txt',
      'calling.txt',
      'features.txt',
      'font.txt',
      'glyph_string.png',
      'graphite.css',
      'guardspace.png',
      'hacking.txt',
      'intro.txt',
      'testing.txt',
      '../tests/examples/simple.c',
      '../tests/examples/freetype.c',
      '../tests/examples/cluster.c',
      '../tests/examples/linebreak.c',
      '../tests/examples/features.c')
    gtf_deps = files('graphite.css', 'OpCodes.txt')

    man_html = custom_target('manual.html',
        output: 'manual.html',
        input: 'manual.txt',
        depend_files: manual_deps,
        install: true,
        install_dir: installdir,
        command: a2x_html)
    gtf_html = custom_target('gtf.html',
        output: 'GTF.html',
        input: 'GTF.txt',
        depend_files: gtf_deps,
        install: true,
        install_dir: installdir,
        command: a2x_html)

    if dblatex.found()
        a2x_pdf = [a2x,
            '--icons',
            '--destination-dir=@OUTDIR@', '@INPUT@']
        custom_target('manual.pdf',
            output: 'manual.pdf',
            input: 'manual.txt',
            depends: man_html,
            install: true,
            install_dir: installdir,
            command: a2x_pdf)
        custom_target('gtf.pdf',
            output: 'GTF.pdf',
            input: 'GTF.txt',
            depends: gtf_html,
            install: true,
            install_dir: installdir,
            command: a2x_pdf)
    endif
endif

# if doxygen.found() and pdflatex.found()
#     make = find_program('make')
#     doxy_conf = get_option('GRAPHITE2_DOXYGEN_CONFIG')
#     doxy = run_target('doxygen/latex/refman.tex',
#         command: [doxygen, doxy_conf+'.cfg'])
#     run_target('doxygen/latex/refman.pdf',
#         command: [make, '-C', 'doxygen/latex'])
# #    install_data('doxygen/latex/refman.pdf', install_dir: installdir)
# endif
