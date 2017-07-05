ft = dependency('freetype2', required: false)

examples = [
  ['simple', 'simple.c', 'Padauk', ['Hello World!']],
  ['features', 'features.c', 'Padauk', []],
  ['clusters', 'cluster.c', 'Padauk', ['စက္ခုန္ဒြေ']],
  ['linebreak', 'linebreak.c', 'charis_r_gr',
    ['120', '"This is a long test line that goes on and on and on"']]
]

if ft.found()
  examples += [['freetype', 'freetype.c', 'Padauk', ['Hello World!']]]
endif

foreach t : examples
  e = executable(t[0], t[1], dependencies: [gr2, ft], c_args: '-Wno-unused')
  test(t[0], e , args: [join_paths(fonts, t[2] + '.ttf')] + t[3])
endforeach
