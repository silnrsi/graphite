lib_version = '3.0.1'

defns=[]
extra_src=[]

if segcache
  extra_src += ['SegCache.cpp', 'SegCacheEntry.cpp', 'SegCacheStore.cpp']
else
  defns += ['-DGRAPHITE2_NSEGCACHE']
endif

if fileface
  extra_src += ['FileFace.cpp']
else
  defns += ['-DGRAPHITE2_NFILEFACE']
endif

if tracing
  extra_src += ['json.cpp']
else
  defns += ['-DGRAPHITE2_NTRACING']
endif

if telemtry
  defns += ['-DGRAPHITE2_TELEMETRY']
endif

if build_static
  defns += ['-DGRAPHITE2_STATIC']
endif

if compiler.get_id() == 'gcc' or compiler.get_id() == 'clang'
  warns = [
     '-Wctor-dtor-privacy',
     '-Wdouble-promotion',
     '-Wendif-labels',
     '-Wextra',
     '-Wno-unknown-pragmas',
     '-Wshadow'
  ]
  flags = [
    '-fno-exceptions',
    '-fno-rtti',
     '-fno-stack-protector',
     '-fvisibility=hidden',
     '-fvisibility-inlines-hidden'
  ]

  if compiler.get_id() == 'clang'
    warns += ['-Wimplicit-fallthrough','-Wno-double-promotion']
  endif

  if host_machine.system() == 'darwin'
    warns += ['-Wno-ctor-dtor-privacy', '-Wno-non-virtual-dtor ']
  endif

  if host_machine.cpu_family() == 'x86'
    flags += ['-mfpmath=sse', '-msse2 ']
  endif
endif

if compiler.get_id() == 'msvc'
  warns = []
  flags = []
  defns = [
    '-D_SCL_SECURE_NO_WARNINGS',
    '-D_CRT_SECURE_NO_WARNINGS',
    '-DUNICODE',
    '-DGRAPHITE2_EXPORTING'
  ]
endif

libc = compiler.find_library('c')
libgcc = compiler.find_library('gcc')

pkg = import('pkgconfig')
pkg.generate(
    name: 'Graphite2',
    version: lib_version,
    description: 'Font rendering engine for Complex Scripts')

vm_machine = '_'.join([vm_type,'machine.cpp'])
lib = library('graphite2',
    vm_machine,
    'gr_char_info.cpp',
    'gr_features.cpp',
    'gr_face.cpp',
    'gr_font.cpp',
    'gr_logging.cpp',
    'gr_segment.cpp',
    'gr_slot.cpp',
    'CachedFace.cpp',
    'CmapCache.cpp',
    'Code.cpp',
    'Collider.cpp',
    'Decompressor.cpp',
    'Face.cpp',
    'FeatureMap.cpp',
    'Font.cpp',
    'GlyphFace.cpp',
    'GlyphCache.cpp',
    'Intervals.cpp',
    'Justifier.cpp',
    'NameTable.cpp',
    'Pass.cpp',
    'Position.cpp',
    'Segment.cpp',
    'Silf.cpp',
    'Slot.cpp',
    'Sparse.cpp',
    'TtfUtil.cpp',
    'UtfCodec.cpp',
    extra_src,
    cpp_args: defns + warns + flags,
    link_args: ['-nodefaultlibs'],
    version: lib_version,
    include_directories: inc,
    dependencies: [libc, libgcc],
    install: true)

gr2 = declare_dependency(link_with: lib, include_directories: inc)
