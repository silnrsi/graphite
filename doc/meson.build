a2x      = find_program('a2x',      required: false)
doxygen  = find_program('doxygen',  required: false)
pdflatex = find_program('pdflatex', required: false)

installdir = join_paths(get_option('datadir'), 'doc', meson.project_name())
installdocs = get_option('enable-docs')

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

    man_html = custom_target('manual.html', command: a2x_html,
        depend_files: manual_deps,
        input: 'manual.txt',
        output: 'manual.html',
        install: installdocs,
        install_dir: installdir)
    gtf_html = custom_target('GTF.html', command: a2x_html,
        depend_files: gtf_deps,
        input: 'GTF.txt',
        output: 'GTF.html',
        install: installdocs,
        install_dir: installdir)
    if installdocs
      install_data('glyph_string.png', 'graphite.css','guardspace.png',
          install_dir: installdir)
    endif

    dblatex  = find_program('dblatex',  required: false)
    if dblatex.found()
        a2x_pdf = [a2x,
            '--icons',
            '--destination-dir=@OUTDIR@', '@INPUT@']
        custom_target('manual.pdf', command: a2x_pdf,
            depends: man_html,
            input: 'manual.txt',
            output: 'manual.pdf',
            install: installdocs,
            install_dir: installdir)
        custom_target('gtf.pdf', command: a2x_pdf,
            depends: gtf_html,
            input: 'GTF.txt',
            output: 'GTF.pdf',
            install: installdocs,
            install_dir: installdir)
    endif
endif

if doxygen.found() and pdflatex.found()
  make = find_program('make')

  doxy_conf = configuration_data()
  doxy_conf.set('project_name', meson.project_name())
  doxy_conf.set('project_number', meson.project_version())
  doxy_conf.set('source_root', meson.source_root())
  doxy_conf.set('build_root', meson.current_build_dir())
  doxy_conf.set('have_dot', find_program('dot',
                                required: false).found() ? 'YES' : 'NO')

  doxy_cfg = get_option('GRAPHITE2_DOXYGEN_CONFIG') + '.cfg'
  doxyfile = configure_file(input: doxy_cfg+'.in',
      output: doxy_cfg,
      configuration: doxy_conf,
      install: false)

  api_docs = custom_target('html',
      input: doxyfile,
      output: ['html', 'refman.tex'],
      depend_files: headers,
      command: [doxygen, '@INPUT@'],
      install: installdocs,
      install_dir: installdir)

  api_pdf = custom_target('refman.pdf',
      output: 'refman.pdf',
      depends: api_docs,
      command: [make, '-C', 'doc'],
      install: installdocs,
      install_dir: installdir)
endif
