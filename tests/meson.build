cmpfiles = ['-c', 'import sys, filecmp; exit(not filecmp.cmp(*sys.argv[1:]))']
texts = join_paths(meson.source_root(), 'tests', 'texts')
fonts = join_paths(meson.source_root(), 'tests', 'fonts')
stnds = join_paths(meson.source_root(), 'tests', 'standards')

libsrc = include_directories(join_paths('../', 'src'))
comparerenderer = get_option('GRAPHITE2_COMPARE_RENDERER') and segcache and fileface

if comparerenderer
  subdir('comparerenderer')
endif
subdir('endian')
subdir('bittwiddling')
if fileface
  subdir('examples')
endif
subdir('featuremap')
subdir('grlist')
subdir('json')
subdir('nametabletest')
if segcache and fileface
  subdir('segcache')
endif
subdir('sparsetest')
subdir('utftest')
if fileface
  subdir('vm')
endif
#subdir('fuzz-tests')

if segcache and fileface
  fonttests = [
    ['padauk1', 'Padauk', ['1015', '102F', '100F', '1039', '100F', '1031', '1038']],
    ['padauk2', 'Padauk', ['1000', '103C', '102D', '102F']],
    ['padauk3', 'Padauk', ['101e', '1004', '103a', '1039', '1001', '103b', '102d', '102f', '1004', '103a', '1038']],
    ['padauk4', 'Padauk', ['1005', '1000', '1039', '1000', '1030']],
    ['padauk5', 'Padauk', ['1000', '103c', '1031', '102c', '1004', '1037', '103a']],
    ['padauk6', 'Padauk', ['1000', '102D', '1005', '1039', '1006', '102C']],
    # padauk7 can cause an infinite loop, though the text is miss-spelt
    ['padauk7', 'Padauk', ['1017', '1014', '103c', '103d', '102f']],
    ['padauk8', 'Padauk', ['1004', '103A', '1039', '1005']],
    ['padauk9', 'Padauk', ['1004', '103A', '1039']],
    ['padauk10', 'Padauk', ['1004', '103D', '1000', '103A', '-feat', 'kdot=1,wtri=1']],
    ['padauk11', 'Padauk', ['100B', '1039', '100C', '1031', '102C']],
    ['padauk12', 'Padauk', ['0048', '0065', '006C', '006C', '006F', '0020', '004D', '0075', '006D', '-j', '107']],
    ['scher1', 'Scheherazadegr', ['0628', '0628', '064E', '0644', '064E', '0654', '0627', '064E', '-rtl']],
    ['scher2', 'Scheherazadegr', ['0627', '0644', '0625', '0639', '0644', '0627', '0646', '-rtl']],
    ['scher3', 'Scheherazadegr', ['0627', '0031', '0032', '002D', '0034', '0035', '0627', '-rtl']],
    ['scher4', 'Scheherazadegr', ['0627', '0653', '06AF', '-rtl']],
    ['scher5', 'Scheherazadegr_noglyfs', ['0627', '0653', '06AF', '-rtl']],
    ['charis1', 'charis_r_gr', ['0069', '02E6', '02E8', '02E5']],
    ['charis2', 'charis_r_gr', ['1D510', '0041', '1D513']],
    ['charis3', 'charis_r_gr', ['0054', '0069', '1ec3', '0075', '-feat', 'lang=vie']],
    ['charis4', 'charis_r_gr', ['006b', '0361', '070']],
    ['charis5', 'charis_r_gr', ['0020', '006C', '0325', '0065']],
    ['charis6', 'charis_r_gr', ['0048', '0065', '006C', '006C', '006F', '0020', '004D', '0075', '006D', '-j', '107']],
    ['charis7', 'charis_fast', ['0049', '0065', '006C', '006C', '006F']],
    ['charis8', 'charis_r_gr', ['0054', '0069', '1ec3', '0075', '-feat', '"lang=vi  "']],
    ['magyar1', 'MagyarLinLibertineG', ['0031', '0035', '-feat', '210=36']],
    ['magyar2', 'MagyarLinLibertineG', ['0031', '0030', '-feat', '210=200']],
    ['magyar3', 'MagyarLinLibertineG', ['0066', '0069', '0066', '0074', '0079', '002d', '0066', '0069', '0076', '0065', '-feat', '209=3']],
    ['grtest1', 'grtest1gr', ['0062', '0061', '0061', '0061', '0061', '0061', '0061', '0062', '0061']],
    ['general1', 'general', ['0E01', '0062']],
    ['piglatin1', 'PigLatinBenchmark_v3', ['0068', '0065', '006C', '006C', '006F']],
  ]

  foreach t : fonttests
    testname = t[0]
    fontfile = t[1] + '.ttf'
    args = t[2]

    test(testname, gr2fonttest, args: [
        '-trace', testname + '.json',
        '-log', testname + '.log',
        join_paths(fonts, fontfile),
        '-codes'] + args,
      timeout: 3)

    test(testname + 'Output', python, is_parallel: false, args:
        cmpfiles + [testname + '.log', join_paths(stnds, testname + '.log')])
    if tracing
      test(testname + 'Debug', python, args: [
          join_paths(meson.source_root(), 'tests', 'jsoncmp'),
          testname + '.json',
          join_paths(stnds, testname + '.json')
        ])
    endif
  endforeach

  feattests = [
    ['padauk_feat', 'Padauk'],
    ['charis_feat', 'charis_r_gr'],
    ['scher_feat', 'Scheherazadegr'],
  ]

  foreach t : feattests
    testname = t[0]
    fontfile = t[1] + '.ttf'

    test(testname, gr2fonttest, args: [
        '-log', testname + '.log',
        join_paths(fonts, fontfile)
      ],
      timeout: 3)
    test(testname + 'Output', python, is_parallel: false, args:
        cmpfiles + [
          testname + '.log',
          join_paths(stnds, testname + '.log')
        ])
  endforeach
endif

cmptests = [
  ['padaukcmp1', 'Padauk', 'my_HeadwordSyllables', []],
  ['chariscmp1', 'charis_r_gr', 'udhr_eng', []],
  ['chariscmp2', 'charis_r_gr', 'udhr_yor', []],
  ['annacmp1', 'Annapurnarc2', 'udhr_nep', []],
  ['schercmp1', 'Scheherazadegr', 'udhr_arb', ['-r']],
  ['awamicmp1', 'Awami_test', 'awami_tests', ['-r', '-e', '1']],
  ['awamicmp2', 'Awami_compressed_test', 'awami_tests', ['-r', '-e', '1']],
]

if fileface and not build_static
  foreach t : cmptests
    testname = t[0]
    fontfile = t[1] + '.ttf'
    txtfile = t[2] + '.txt'
    args = t[3]

    test(testname, python2, args: [
        join_paths(meson.source_root(), 'tests', 'fnttxtrender'),
        '--graphite_library=' + lib.full_path(),
        '-t', join_paths(texts, txtfile),
        '-o', testname + '.json',
        '-c', join_paths(stnds, testname + '.json')] + args + [
        join_paths(fonts, fontfile)
      ])
  endforeach
endif
