e = executable('grsegcachetest', 'segcachetest.cpp',
    include_directories: [inc, libsrc],
    objects: lib.extract_all_objects())
test('grsegcachetest', e, args: join_paths(fonts, 'Padauk.ttf'), timeout: 3)
