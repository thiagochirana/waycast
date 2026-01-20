project('waycast', 'c',
  version: '0.1.1',
  default_options: ['c_std=c11']
)

glibdep = dependency('glib-2.0')
raylibdep = dependency('raylib')

executable('waycast',
  [
    'src/main.c',
    'src/ConfigLoader.c',
    'src/ThemeManager.c',
    'src/UIManager.c',
    'src/ui/ui_app_data.c',
    'src/ui/ui_search_bar.c',
    'src/ui/ui_app_grid.c'
  ],
  include_directories: include_directories('include'),
  dependencies: [glibdep, raylibdep],
  install: true
)
