e = executable('featuremaptest', 'featuremaptest.cpp',
    include_directories: [inc, libsrc],
    objects: lib.extract_all_objects())
test('featuremaptest', e, args: join_paths(fonts, 'tiny.ttf'), timeout: 3)
