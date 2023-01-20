# Generate configuration header
config_h = configuration_data()
config_h.set_quoted('VERSION', meson.project_version())
configure_file(output: 'config.h', configuration: config_h)

input_emulator_sources = [
  'main.c',
  'touch.c',
  'keyboard.c',
  'mouse.c',
  'service.c',
  'message.c',
  'options.c',
  'signals.c',
  'filesystem.c',
  'event.c',
  'misc.c',
]

input_emulator_c_args = ['-Wno-unused-result']

# Test for rt library support
compiler = meson.get_compiler('c')
rt_dep = compiler.find_library('rt', required : true)

input_emulator_dep = [
  rt_dep,
]

# Test for Lua support
enable_lua = get_option('lua')
if enable_lua
  foreach name: ['lua-5.4', 'lua-5.3', 'lua-5.2', 'lua-5.1', 'lua']
    lua_dep = dependency(name, version: '>=5.1', required: true)
  if lua_dep.found()
    input_emulator_c_args += '-DHAVE_LUA'
    input_emulator_dep += lua_dep
    break
  endif
endforeach
endif

executable('input-emulator',
  input_emulator_sources,
  c_args: input_emulator_c_args,
  dependencies: input_emulator_dep,
  install: true )

