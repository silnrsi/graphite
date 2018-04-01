eo = lib.extract_all_objects()

vm_test_common = static_library('vm-test-common', 'basic_test.cpp',
    include_directories: [inc, libsrc],
    objects: eo,
    cpp_args: '-DGRAPHITE2_NTRACING')

test_args = [join_paths(fonts, 'small.ttf'), '1']
ct = executable('vm-test-call',
    join_paths(meson.source_root(), 'src', 'call_machine.cpp'),
    include_directories: [inc, libsrc],
    link_with: vm_test_common)
test('vm-test-call-threading', ct, args: test_args)

if ['gcc','clang'].contains(compiler.get_id())
  dt = executable('vm-test-direct',
      join_paths(meson.source_root(), 'src', 'direct_machine.cpp'),
      include_directories: [inc, libsrc],
      link_with: vm_test_common)
  test('vm-test-direct-threading', dt, args: test_args)
endif
