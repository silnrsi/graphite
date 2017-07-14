project('graphite2', 'cpp','c',
        version: '1.3.9',
        license: ['GPL2+', 'MPL'],
        default_options : ['buildtype=release'])

compiler = meson.get_compiler('cpp')

segcache = not get_option('GRAPHITE2_NSEGCACHE')
fileface = not get_option('GRAPHITE2_NFILEFACE')
tracing  = not get_option('GRAPHITE2_NTRACING')
telemtry = get_option('GRAPHITE2_TELEMETRY')
vm_type = get_option('GRAPHITE2_VM_TYPE')
build_static = get_option('default_library') == 'static'
if vm_type == 'auto'
    vm_type = get_option('buildtype') == 'debug' ? 'call' : 'direct'
endif

if vm_type == 'direct' and not ['gcc','clang'].contains(compiler.get_id())
    message('VM Machine type direct can only be built using GCC or Clang.')
    vm_type = 'call'
endif

message('''
Segment Cache support: @0@
File Face support:     @1@
Tracing support:       @2@
Memory telemtry:       @3@
VM Machine type:       @4@
'''.format(segcache, fileface, tracing, telemtry, vm_type))

py = import('python3')
python = py.find_python()
python2 = find_program('python2')
if not python.found()
  python = python2
endif

inc = include_directories('include', is_system: true)

subdir('include/graphite2')
subdir('src')
if segcache or fileface
    subdir('gr2fonttest')
endif
subdir('tests')
subdir('doc')
