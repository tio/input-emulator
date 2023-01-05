config_h = configuration_data()
config_h.set_quoted('VERSION', meson.project_version())
configure_file(output: 'config.h', configuration: config_h)

keyboard_simulator_sources = [
  'keyboard-simulator.c'
]

foreach name: ['lua-5.4', 'lua-5.3', 'lua-5.2', 'lua-5.1', 'lua']
  lua_dep = dependency(name, version: '>=5.1', required: false)
  if lua_dep.found()
    break
  endif
endforeach
if not lua_dep.found()
  error('Lua could not be found!')
endif

compiler = meson.get_compiler('c')
rt_dep = compiler.find_library('rt', required : true)

keyboard_simulator_dep = [
  rt_dep,
  lua_dep,
]

keyboard_simulator_c_args = ['-Wno-unused-result']

executable('keyboard-simulator',
  keyboard_simulator_sources,
  c_args: keyboard_simulator_c_args,
  dependencies: keyboard_simulator_dep,
  install: true )

# subdir('bash-completion')
