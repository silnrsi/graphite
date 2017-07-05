e = executable('jsontest', 'jsontest.cpp',
    include_directories: [inc, libsrc],
    objects: lib.extract_objects('json.cpp'))
test('jsontest', e, args: 'jsontest.log')
test('jsontestOutput', python, is_parallel: false, args:
    cmpfiles + ['jsontest.log', join_paths(stnds, 'jsontest.log')])
