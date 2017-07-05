ft = dependency('freetype2')
hb = dependency('harfbuzz', required: false)
le = dependency('icu-le', required: false)
uc = dependency('icu-uc', required: le.found())

e = executable('comparerenderer', 'CompareRenderer.cpp',
      dependencies: [gr2, ft, hb, le, uc])

tests = [
  ['charis', 'charis_r_gr', 'udhr_eng', ''],
  ['padauk', 'Padauk', 'my_HeadwordSyllables', ''],
  ['padauk_ulon', 'Padauk', 'my_HeadwordSyllables', '--features ulon=1'],
  ['charis_yor', 'charis_r_gr', 'udhr_yor', ''],
  ['piglatin', 'PigLatinBenchmark_v3', 'udhr_eng', ''],
  # silgraphite and gr2 bidi algorithms don't quite agree enough
  #  ['scheherazade_arb', 'Scheherazadegr', 'udhr_arb', '--rtl']
  ['annapurna_nep', 'Annapurnarc2', 'udhr_nep', '']
]

foreach t : tests
  test(t[0] + '_comparision', e , args: [
        '-n',
        '--tolerance', '0.0001',
        '-s', '12',
        '-v',
        '-f', join_paths(fonts, t[1] + '.ttf'),
        '-t', join_paths(texts, t[2] + '.txt'),
        '-l', t[0] + '.log'
      ] + t[3].split(),
      timeout: 3)
  test(t[0] + '_cmpOutput', python, is_parallel: false, args:
      cmpfiles + [t[0] + '.log', join_paths(stnds, t[0] + '.log')])
endforeach
