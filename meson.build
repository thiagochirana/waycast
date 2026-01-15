project('waycast', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11']
)

glibdep = dependency('glib-2.0')
raylibdep = dependency('raylib')

executable('waycast',
  ['src/main.c', 'src/ConfigLoader.c', 'src/ThemeManager.c', 'src/UIManager.c'],
  include_directories: include_directories('include'),
  dependencies: [glibdep, raylibdep],
  install: true
)
