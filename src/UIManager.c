#include "UIManager.h"
#include <glib.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    char *exec;
    char *icon;
} App;

typedef struct {
    Color background;
    Color panel;
    Color panel_border;
    Color text;
    Color muted_text;
    Color highlight;
    Color highlight_border;
} Palette;

static GPtrArray *apps = NULL;

static void free_app(gpointer data) {
    App *app = (App *)data;
    if (!app)
        return;

    g_free(app->name);
    g_free(app->exec);
    g_free(app->icon);
    g_free(app);
}

static void load_apps(void) {
    if (!apps)
        apps = g_ptr_array_new_with_free_func(free_app);
    else
        g_ptr_array_set_size(apps, 0);

    const char *dirs[] = {
        "/usr/share/applications",
        g_build_filename(g_get_home_dir(), ".local", "share", "applications", NULL)
    };

    for (guint i = 0; i < G_N_ELEMENTS(dirs); i++) {
        const char *dir_path = dirs[i];
        if (!dir_path || !g_file_test(dir_path, G_FILE_TEST_IS_DIR)) {
            if (i == 1)
                g_free((gpointer)dir_path);
            continue;
        }

        GDir *dir = g_dir_open(dir_path, 0, NULL);
        if (!dir) {
            if (i == 1)
                g_free((gpointer)dir_path);
            continue;
        }

        const gchar *entry_name;
        while ((entry_name = g_dir_read_name(dir)) != NULL) {
            if (!g_str_has_suffix(entry_name, ".desktop"))
                continue;

            gchar *full_path = g_build_filename(dir_path, entry_name, NULL);
            GKeyFile *file = g_key_file_new();
            if (g_key_file_load_from_file(file, full_path, G_KEY_FILE_NONE, NULL)) {
                gchar *name = g_key_file_get_string(file, "Desktop Entry", "Name", NULL);
                gchar *exec = g_key_file_get_string(file, "Desktop Entry", "Exec", NULL);
                gchar *icon = g_key_file_get_string(file, "Desktop Entry", "Icon", NULL);

                if (name && exec) {
                    App *app = g_new0(App, 1);
                    app->name = name;
                    app->exec = exec;
                    app->icon = icon;
                    g_ptr_array_add(apps, app);
                } else {
                    g_free(name);
                    g_free(exec);
                    g_free(icon);
                }
            }
            g_key_file_free(file);
            g_free(full_path);
        }

        g_dir_close(dir);
        if (i == 1)
            g_free((gpointer)dir_path);
    }
}

static void launch_app(const App *app) {
    if (!app || !app->exec)
        return;
    g_spawn_command_line_async(app->exec, NULL);
}

static gchar *casefold_text(const gchar *text) {
    if (!text || !*text)
        return NULL;
    return g_utf8_casefold(text, -1);
}

static void update_filtered_apps(const char *query, GArray *filtered_indices) {
    g_array_set_size(filtered_indices, 0);

    if (!query || query[0] == '\0')
        return;

    gchar *lowered_query = casefold_text(query);
    if (!lowered_query)
        return;

    for (guint i = 0; i < apps->len; i++) {
        App *app = g_ptr_array_index(apps, i);
        if (!app || !app->name)
            continue;

        gchar *lowered_name = casefold_text(app->name);
        if (lowered_name && g_strstr_len(lowered_name, -1, lowered_query)) {
            g_array_append_val(filtered_indices, i);
        }
        g_free(lowered_name);
    }

    g_free(lowered_query);
}

static Palette palette_from_theme(const char *theme_name) {
    const bool light = (theme_name && strcmp(theme_name, "light") == 0);

    if (light) {
        return (Palette){
            .background = (Color){245, 243, 238, 255},
            .panel = (Color){255, 255, 255, 255},
            .panel_border = (Color){214, 206, 197, 255},
            .text = (Color){26, 25, 24, 255},
            .muted_text = (Color){104, 98, 91, 255},
            .highlight = (Color){238, 228, 210, 255},
            .highlight_border = (Color){190, 176, 160, 255}
        };
    }

    return (Palette){
        .background = (Color){20, 22, 28, 255},
        .panel = (Color){31, 33, 41, 255},
        .panel_border = (Color){60, 66, 78, 255},
        .text = (Color){240, 236, 229, 255},
        .muted_text = (Color){156, 152, 145, 255},
        .highlight = (Color){52, 58, 72, 255},
        .highlight_border = (Color){120, 126, 140, 255}
    };
}

static void apply_backspace_utf8(char *buffer) {
    if (!buffer || buffer[0] == '\0')
        return;

    char *end = buffer + strlen(buffer);
    char *prev = g_utf8_find_prev_char(buffer, end);
    if (!prev)
        return;

    *prev = '\0';
}

void ui_manager_start(const Config *config) {
    const int width = (config && config->window.width > 0) ? config->window.width : 600;
    const int height = (config && config->window.height > 0) ? config->window.height : 400;

    load_apps();

    Palette palette = palette_from_theme((config && config->theme) ? config->theme : "default");

    InitWindow(width, height, "Waycast");
    SetTargetFPS(60);
    Font ui_font = LoadFontEx("fonts/SFMono-Regular.otf", 32, NULL, 0);
    bool has_font = (ui_font.texture.id != 0);

    const int margin = 16;
    const int search_height = 44;
    const int cell_width = 140;
    const int cell_height = 108;
    const int cell_gap = 12;

    char search_text[256] = {0};
    bool search_dirty = true;

    GArray *filtered_indices = g_array_new(FALSE, FALSE, sizeof(guint));
    int selected_index = -1;
    float scroll_y = 0.0f;

    while (!WindowShouldClose()) {
        bool text_changed = false;

        int key = GetCharPressed();
        while (key > 0) {
            if ((unsigned int)key >= 32 && (unsigned int)key < 127) {
                size_t len = strlen(search_text);
                if (len + 1 < sizeof(search_text)) {
                    search_text[len] = (char)key;
                    search_text[len + 1] = '\0';
                    text_changed = true;
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            apply_backspace_utf8(search_text);
            text_changed = true;
        }

        if (IsKeyPressed(KEY_ESCAPE))
            break;

        if (text_changed) {
            search_dirty = true;
            scroll_y = 0.0f;
        }

        if (search_dirty) {
            update_filtered_apps(search_text, filtered_indices);
            if (filtered_indices->len > 0)
                selected_index = 0;
            else
                selected_index = -1;
            search_dirty = false;
        }

        int available_width = width - margin * 2;
        int grid_cols = (available_width + cell_gap) / (cell_width + cell_gap);
        if (grid_cols < 1)
            grid_cols = 1;

        if (IsKeyPressed(KEY_RIGHT) && selected_index >= 0) {
            if (selected_index + 1 < (int)filtered_indices->len)
                selected_index++;
        }
        if (IsKeyPressed(KEY_LEFT) && selected_index >= 0) {
            if (selected_index - 1 >= 0)
                selected_index--;
        }
        if (IsKeyPressed(KEY_DOWN) && selected_index >= 0) {
            int next = selected_index + grid_cols;
            if (next < (int)filtered_indices->len)
                selected_index = next;
        }
        if (IsKeyPressed(KEY_UP) && selected_index >= 0) {
            int next = selected_index - grid_cols;
            if (next >= 0)
                selected_index = next;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            if (selected_index >= 0 && selected_index < (int)filtered_indices->len) {
                guint app_idx = g_array_index(filtered_indices, guint, selected_index);
                App *app = g_ptr_array_index(apps, app_idx);
                launch_app(app);
                break;
            }
        }

        int content_rows = (filtered_indices->len + grid_cols - 1) / grid_cols;
        float content_height = content_rows * (cell_height + cell_gap) - cell_gap;
        float viewport_height = height - margin * 2 - search_height - 8;
        if (viewport_height < 0)
            viewport_height = 0;

        float max_scroll = content_height - viewport_height;
        if (max_scroll < 0)
            max_scroll = 0;

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            scroll_y -= wheel * 32.0f;
            if (scroll_y < 0.0f)
                scroll_y = 0.0f;
            if (scroll_y > max_scroll)
                scroll_y = max_scroll;
        }

        if (selected_index >= 0) {
            int row = selected_index / grid_cols;
            float item_top = row * (cell_height + cell_gap);
            float item_bottom = item_top + cell_height;
            if (item_top < scroll_y)
                scroll_y = item_top;
            else if (item_bottom > scroll_y + viewport_height)
                scroll_y = item_bottom - viewport_height;
        }

        BeginDrawing();
        ClearBackground(palette.background);

        Rectangle search_rect = {
            (float)margin,
            (float)margin,
            (float)(width - margin * 2),
            (float)search_height
        };
        DrawRectangleRec(search_rect, palette.panel);
        DrawRectangleLinesEx(search_rect, 1.0f, palette.panel_border);

    const int font_size = 18;
    int text_padding = 12;
    if (search_text[0] != '\0') {
            Vector2 pos = {(float)(margin + text_padding), (float)(margin + (search_height - font_size) / 2)};
            if (has_font)
                DrawTextEx(ui_font, search_text, pos, (float)font_size, 0.0f, palette.text);
            else
                DrawText(search_text, (int)pos.x, (int)pos.y, font_size, palette.text);
        } else {
            Vector2 pos = {(float)(margin + text_padding), (float)(margin + (search_height - font_size) / 2)};
            if (has_font)
                DrawTextEx(ui_font, "Search apps...", pos, (float)font_size, 0.0f, palette.muted_text);
            else
                DrawText("Search apps...", (int)pos.x, (int)pos.y, font_size, palette.muted_text);
        }

        float grid_top = margin + search_height + 8;
        float grid_left = margin;
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        for (guint i = 0; i < filtered_indices->len; i++) {
            int row = i / grid_cols;
            int col = i % grid_cols;
            float x = grid_left + col * (cell_width + cell_gap);
            float y = grid_top + row * (cell_height + cell_gap) - scroll_y;

            if (y + cell_height < grid_top || y > grid_top + viewport_height)
                continue;

            Rectangle cell = {x, y, (float)cell_width, (float)cell_height};
            bool is_selected = ((int)i == selected_index);
            Color fill = is_selected ? palette.highlight : palette.panel;
            Color border = is_selected ? palette.highlight_border : palette.panel_border;

            DrawRectangleRec(cell, fill);
            DrawRectangleLinesEx(cell, 1.0f, border);

            guint app_idx = g_array_index(filtered_indices, guint, i);
            App *app = g_ptr_array_index(apps, app_idx);
            const char *name = app ? app->name : "";

            int name_font = 16;
            int max_width = cell_width - 16;
            const char *trimmed = TextSubtext(name, 0, (int)strlen(name));
            float measured = has_font
                ? MeasureTextEx(ui_font, trimmed, (float)name_font, 0.0f).x
                : (float)MeasureText(trimmed, name_font);
            if (measured > max_width) {
                int chars = (int)strlen(name);
                while (chars > 0) {
                    const char *candidate = TextSubtext(name, 0, chars);
                    float candidate_width = has_font
                        ? MeasureTextEx(ui_font, candidate, (float)name_font, 0.0f).x
                        : (float)MeasureText(candidate, name_font);
                    if (candidate_width <= max_width)
                        break;
                    chars--;
                }
                if (chars > 3) {
                    char short_name[128];
                    if (chars > (int)sizeof(short_name) - 4)
                        chars = (int)sizeof(short_name) - 4;
                    memcpy(short_name, name, (size_t)chars);
                    memcpy(short_name + chars, "...", 4);
                    Vector2 pos = {(float)((int)x + 8), (float)((int)y + cell_height / 2 - 8)};
                    if (has_font)
                        DrawTextEx(ui_font, short_name, pos, (float)name_font, 0.0f, palette.text);
                    else
                        DrawText(short_name, (int)pos.x, (int)pos.y, name_font, palette.text);
                } else {
                    Vector2 pos = {(float)((int)x + 8), (float)((int)y + cell_height / 2 - 8)};
                    if (has_font)
                        DrawTextEx(ui_font, name, pos, (float)name_font, 0.0f, palette.text);
                    else
                        DrawText(name, (int)pos.x, (int)pos.y, name_font, palette.text);
                }
            } else {
                Vector2 pos = {(float)((int)x + 8), (float)((int)y + cell_height / 2 - 8)};
                if (has_font)
                    DrawTextEx(ui_font, name, pos, (float)name_font, 0.0f, palette.text);
                else
                    DrawText(name, (int)pos.x, (int)pos.y, name_font, palette.text);
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec((Vector2){(float)mouse_x, (float)mouse_y}, cell)) {
                selected_index = (int)i;
            }
        }

        EndDrawing();
    }

    if (has_font)
        UnloadFont(ui_font);
    g_array_free(filtered_indices, TRUE);
    if (apps)
        g_ptr_array_free(apps, TRUE);
    apps = NULL;

    CloseWindow();
}
