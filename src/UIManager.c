#include "UIManager.h"
#include "ui/ui_app_data.h"
#include "ui/ui_app_grid.h"
#include "ui/ui_search_bar.h"
#include <raylib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    Color background;
    Color panel;
    Color panel_border;
    Color text;
    Color muted_text;
    Color highlight;
    Color highlight_border;
} Palette;

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
        .background = (Color){0, 0, 0, 255},
        .panel = (Color){6, 6, 6, 255},
        .panel_border = (Color){255, 255, 255, 255},
        .text = (Color){255, 255, 255, 255},
        .muted_text = (Color){200, 200, 200, 255},
        .highlight = (Color){20, 20, 20, 255},
        .highlight_border = (Color){255, 255, 255, 255}
    };
}

static void center_window(int width, int height) {
    const int monitor = GetCurrentMonitor();
    int x = (GetMonitorWidth(monitor) - width) / 2;
    int y = (GetMonitorHeight(monitor) - height) / 2;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    SetWindowPosition(x, y);
}

void ui_manager_start(const Config *config) {
    (void)config;
    const int initial_width = 1000;
    const int initial_height = 600;

    UIAppData data;
    ui_app_data_init(&data);
    ui_app_data_load(&data);

    UISearchBar search;
    ui_search_bar_init(&search);

    UIAppGrid grid;
    ui_app_grid_init(&grid);

    Palette palette = palette_from_theme("default");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(initial_width, initial_height, "Waycast");
    center_window(initial_width, initial_height);
    SetTargetFPS(60);

    Font ui_font = LoadFontEx("fonts/SFMono-Regular.otf", 32, NULL, 0);
    bool has_font = (ui_font.texture.id != 0);

    const int margin = 16;
    const int search_height = 44;
    const int cell_width = 140;
    const int cell_height = 108;
    const int cell_gap = 12;

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            center_window(GetScreenWidth(), GetScreenHeight());
        }

        bool should_close = false;
        ui_search_bar_handle_input(&search, &should_close);
        if (should_close)
            break;

        if (search.dirty) {
            ui_app_data_filter(&data, search.text);
            ui_app_grid_reset(&grid, ui_app_data_filtered_count(&data));
            search.dirty = false;
        }

        const int width = GetScreenWidth();
        const int height = GetScreenHeight();
        int available_width = width - margin * 2;
        int cols = ui_app_grid_columns(available_width, cell_width, cell_gap);
        float grid_top = margin + search_height + 8.0f;
        float viewport_height = (float)height - margin * 2 - search_height - 8.0f;
        if (viewport_height < 0.0f)
            viewport_height = 0.0f;

        guint filtered_count = ui_app_data_filtered_count(&data);
        float content_height = ui_app_grid_content_height(filtered_count, cols, cell_height, cell_gap);
        float max_scroll = content_height - viewport_height;
        if (max_scroll < 0.0f)
            max_scroll = 0.0f;

        ui_app_grid_handle_navigation(&grid, cols, filtered_count);
        ui_app_grid_handle_scroll(&grid, max_scroll);
        ui_app_grid_ensure_visible(&grid, cols, viewport_height, cell_height, cell_gap);

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            if (grid.selected_index >= 0 && grid.selected_index < (int)filtered_count) {
                guint app_idx = ui_app_data_filtered_index(&data, (guint)grid.selected_index);
                const App *app = ui_app_data_get(&data, app_idx);
                ui_app_launch(app);
                break;
            }
        }

        BeginDrawing();
        ClearBackground(palette.background);

        Rectangle search_rect = {
            (float)margin,
            (float)margin,
            (float)(width - margin * 2),
            (float)search_height
        };
        ui_search_bar_draw(&search, search_rect, ui_font, has_font,
                           palette.text, palette.muted_text,
                           palette.panel, palette.panel_border);

        Rectangle grid_viewport = {
            (float)margin,
            grid_top,
            (float)available_width,
            viewport_height
        };

        ui_app_grid_draw(&grid, &data, grid_viewport, cols, cell_width, cell_height,
                         cell_gap, ui_font, has_font,
                         palette.panel, palette.panel_border,
                         palette.highlight, palette.highlight_border,
                         palette.text);

        EndDrawing();
    }

    if (has_font)
        UnloadFont(ui_font);

    ui_app_data_free(&data);
    CloseWindow();
}
