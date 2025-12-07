project('waycast', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11']
)

gtkdep = dependency('gtk4')
adwdep = dependency('libadwaita-1')

executable('waycast',
  ['src/main.c', 'src/ConfigLoader.c', 'src/ThemeManager.c', 'src/UIManager.c'],
  include_directories: include_directories('include'),
  dependencies: [gtkdep, adwdep],
  install: true
)
