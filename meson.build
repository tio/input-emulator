project('input-emulator', 'c',
    version : '0.7',
    license : [ 'GPL-2.0-or-later'],
    meson_version : '>= 0.53.2',
    default_options : [ 'warning_level=2', 'buildtype=release', 'c_std=gnu11', 'b_lundef=false' ]
)

# The tag date of the project_version(), update when the version bumps.
version_date = '2023-1-17'

subdir('src')
subdir('man')
subdir('bash-completion')
