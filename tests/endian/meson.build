e = executable('endiantest', 'endiantest.cpp',
    include_directories: [inc, libsrc])
test('endian', e, args: '1')
benchmark('endian', e, args: '25')
