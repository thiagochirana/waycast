#pragma once
#include "ui/ui_app_data.h"
#include <raylib.h>

typedef struct {
    int selected_index;
    float scroll_y;
} UIAppGrid;

void ui_app_grid_init(UIAppGrid *grid);
void ui_app_grid_reset(UIAppGrid *grid, guint filtered_count);
int ui_app_grid_columns(int available_width, int cell_width, int cell_gap);
float ui_app_grid_content_height(guint filtered_count, int cols, int cell_height, int cell_gap);
void ui_app_grid_handle_navigation(UIAppGrid *grid, int cols, guint filtered_count);
void ui_app_grid_handle_scroll(UIAppGrid *grid, float max_scroll);
void ui_app_grid_ensure_visible(UIAppGrid *grid, int cols, float viewport_height, int cell_height, int cell_gap);
void ui_app_grid_draw(UIAppGrid *grid,
                      const UIAppData *data,
                      Rectangle viewport,
                      int cols,
                      int cell_width,
                      int cell_height,
                      int cell_gap,
                      Font font,
                      bool has_font,
                      Color panel_color,
                      Color panel_border,
                      Color highlight_color,
                      Color highlight_border,
                      Color text_color);
