# Generate configuration header
config_h = configuration_data()
config_h.set_quoted('VERSION', meson.project_version())
configure_file(output: 'config.h', configuration: config_h)

input_emulator_sources = [
  'main.c',
  'misc.c',
  'touch.c',
  'mouse.c',
  'event.c',
  'print.c',
  'service.c',
  'message.c',
  'options.c',
  'signals.c',
  'keyboard.c'
]

input_emulator_c_args = ['-Wno-unused-result', '-Wno-shadow']

enable_debug = get_option('enable-debug')
if enable_debug
  input_emulator_c_args += '-DDEBUG'
endif

# Test for rt library support
compiler = meson.get_compiler('c')
rt_dep = compiler.find_library('rt', required : true)

input_emulator_dep = [
  rt_dep,
]

executable('input-emulator',
  input_emulator_sources,
  c_args: input_emulator_c_args,
  dependencies: input_emulator_dep,
  install: true )

