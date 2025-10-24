project('waycast', 'cpp',
  version: '0.1.0',
  default_options: ['cpp_std=c++20']
)

gtkdep = dependency('gtk4')
adwdep = dependency('libadwaita-1')
tomldep = dependency('tomlplusplus', required : false)

executable('waycast',
  ['src/main.cpp', 'src/ConfigLoader.cpp', 'src/ThemeManager.cpp', 'src/UIManager.cpp'],
  include_directories: include_directories('include'),
  dependencies: [gtkdep, adwdep, tomldep],
  install: true
)
