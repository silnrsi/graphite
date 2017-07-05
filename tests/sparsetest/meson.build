e = executable('sparsetest', 'sparsetest.cpp',
    include_directories: [inc, libsrc],
    objects: lib.extract_objects('Sparse.cpp'))
test('sparsetest', e, timeout: 3)
