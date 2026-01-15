#pragma once
#include <raylib.h>
#include <stdbool.h>

#define UI_SEARCH_MAX 256

typedef struct {
    char text[UI_SEARCH_MAX];
    bool dirty;
} UISearchBar;

void ui_search_bar_init(UISearchBar *bar);

bool ui_search_bar_handle_input(UISearchBar *bar, bool *should_close);

void ui_search_bar_draw(const UISearchBar *bar,
                        Rectangle rect,
                        Font font,
                        bool has_font,
                        Color text_color,
                        Color muted_color,
                        Color panel_color,
                        Color border_color);
