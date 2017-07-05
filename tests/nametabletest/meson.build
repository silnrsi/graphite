e = executable('nametabletest', 'nametabletest.cpp',
    include_directories: [inc, libsrc],
    objects: lib.extract_objects('NameTable.cpp'))
test('nametabletest', e, timeout: 3)
