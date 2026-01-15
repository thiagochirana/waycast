#include "ui/ui_search_bar.h"
#include <glib.h>
#include <string.h>

static void apply_backspace_utf8(char *buffer) {
    if (!buffer || buffer[0] == '\0')
        return;

    char *end = buffer + strlen(buffer);
    char *prev = g_utf8_find_prev_char(buffer, end);
    if (!prev)
        return;

    *prev = '\0';
}

void ui_search_bar_init(UISearchBar *bar) {
    if (!bar)
        return;
    bar->text[0] = '\0';
    bar->dirty = true;
}

bool ui_search_bar_handle_input(UISearchBar *bar, bool *should_close) {
    if (!bar)
        return false;

    bool text_changed = false;
    int key = GetCharPressed();
    while (key > 0) {
        if ((unsigned int)key >= 32 && (unsigned int)key < 127) {
            size_t len = strlen(bar->text);
            if (len + 1 < sizeof(bar->text)) {
                bar->text[len] = (char)key;
                bar->text[len + 1] = '\0';
                text_changed = true;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        apply_backspace_utf8(bar->text);
        text_changed = true;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (should_close)
            *should_close = true;
    }

    if (text_changed)
        bar->dirty = true;

    return text_changed;
}

void ui_search_bar_draw(const UISearchBar *bar,
                        Rectangle rect,
                        Font font,
                        bool has_font,
                        Color text_color,
                        Color muted_color,
                        Color panel_color,
                        Color border_color) {
    if (!bar)
        return;

    DrawRectangleRec(rect, panel_color);
    DrawRectangleLinesEx(rect, 1.0f, border_color);

    const int font_size = 18;
    const int text_padding = 12;
    const char *text = (bar->text[0] != '\0') ? bar->text : "Search apps...";
    Color color = (bar->text[0] != '\0') ? text_color : muted_color;
    Vector2 pos = {rect.x + text_padding, rect.y + (rect.height - font_size) / 2.0f};

    if (has_font)
        DrawTextEx(font, text, pos, (float)font_size, 0.0f, color);
    else
        DrawText(text, (int)pos.x, (int)pos.y, font_size, color);
}
