#include "ui/ui_app_grid.h"
#include <string.h>

void ui_app_grid_init(UIAppGrid *grid) {
    if (!grid)
        return;
    grid->selected_index = -1;
    grid->scroll_y = 0.0f;
}

void ui_app_grid_reset(UIAppGrid *grid, guint filtered_count) {
    if (!grid)
        return;
    grid->scroll_y = 0.0f;
    grid->selected_index = (filtered_count > 0) ? 0 : -1;
}

int ui_app_grid_columns(int available_width, int cell_width, int cell_gap) {
    int cols = (available_width + cell_gap) / (cell_width + cell_gap);
    if (cols < 1)
        cols = 1;
    return cols;
}

float ui_app_grid_content_height(guint filtered_count, int cols, int cell_height, int cell_gap) {
    if (filtered_count == 0)
        return 0.0f;
    int rows = (filtered_count + (guint)cols - 1) / (guint)cols;
    return rows * (float)(cell_height + cell_gap) - (float)cell_gap;
}

void ui_app_grid_handle_navigation(UIAppGrid *grid, int cols, guint filtered_count) {
    if (!grid || filtered_count == 0)
        return;

    if (IsKeyPressed(KEY_RIGHT) && grid->selected_index >= 0) {
        if (grid->selected_index + 1 < (int)filtered_count)
            grid->selected_index++;
    }
    if (IsKeyPressed(KEY_LEFT) && grid->selected_index >= 0) {
        if (grid->selected_index - 1 >= 0)
            grid->selected_index--;
    }
    if (IsKeyPressed(KEY_DOWN) && grid->selected_index >= 0) {
        int next = grid->selected_index + cols;
        if (next < (int)filtered_count)
            grid->selected_index = next;
    }
    if (IsKeyPressed(KEY_UP) && grid->selected_index >= 0) {
        int next = grid->selected_index - cols;
        if (next >= 0)
            grid->selected_index = next;
    }
}

void ui_app_grid_handle_scroll(UIAppGrid *grid, float max_scroll) {
    if (!grid)
        return;

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        grid->scroll_y -= wheel * 32.0f;
        if (grid->scroll_y < 0.0f)
            grid->scroll_y = 0.0f;
        if (grid->scroll_y > max_scroll)
            grid->scroll_y = max_scroll;
    }
}

void ui_app_grid_ensure_visible(UIAppGrid *grid, int cols, float viewport_height, int cell_height, int cell_gap) {
    if (!grid || grid->selected_index < 0)
        return;

    int row = grid->selected_index / cols;
    float item_top = row * (float)(cell_height + cell_gap);
    float item_bottom = item_top + (float)cell_height;

    if (item_top < grid->scroll_y)
        grid->scroll_y = item_top;
    else if (item_bottom > grid->scroll_y + viewport_height)
        grid->scroll_y = item_bottom - viewport_height;

    if (grid->scroll_y < 0.0f)
        grid->scroll_y = 0.0f;
}

static float measure_text_width(Font font, bool has_font, const char *text, int font_size) {
    if (!text)
        return 0.0f;
    if (has_font)
        return MeasureTextEx(font, text, (float)font_size, 0.0f).x;
    return (float)MeasureText(text, font_size);
}

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
                      Color text_color) {
    if (!grid || !data)
        return;

    guint filtered_count = ui_app_data_filtered_count(data);
    if (filtered_count == 0)
        return;

    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();

    for (guint i = 0; i < filtered_count; i++) {
        int row = (int)i / cols;
        int col = (int)i % cols;
        float x = viewport.x + col * (float)(cell_width + cell_gap);
        float y = viewport.y + row * (float)(cell_height + cell_gap) - grid->scroll_y;

        if (y + cell_height < viewport.y || y > viewport.y + viewport.height)
            continue;

        Rectangle cell = {x, y, (float)cell_width, (float)cell_height};
        bool is_selected = ((int)i == grid->selected_index);
        Color fill = is_selected ? highlight_color : panel_color;
        Color border = is_selected ? highlight_border : panel_border;

        DrawRectangleRec(cell, fill);
        DrawRectangleLinesEx(cell, 1.0f, border);

        guint app_idx = ui_app_data_filtered_index(data, i);
        const App *app = ui_app_data_get(data, app_idx);
        const char *name = app ? app->name : "";

        const int name_font = 16;
        int max_width = cell_width - 16;
        float width = measure_text_width(font, has_font, name, name_font);
        Vector2 pos = {x + 8.0f, y + cell_height / 2.0f - 8.0f};

        if (width > max_width) {
            int chars = (int)strlen(name);
            while (chars > 0) {
                const char *candidate = TextSubtext(name, 0, chars);
                float candidate_width = measure_text_width(font, has_font, candidate, name_font);
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

                if (has_font)
                    DrawTextEx(font, short_name, pos, (float)name_font, 0.0f, text_color);
                else
                    DrawText(short_name, (int)pos.x, (int)pos.y, name_font, text_color);
            } else {
                if (has_font)
                    DrawTextEx(font, name, pos, (float)name_font, 0.0f, text_color);
                else
                    DrawText(name, (int)pos.x, (int)pos.y, name_font, text_color);
            }
        } else {
            if (has_font)
                DrawTextEx(font, name, pos, (float)name_font, 0.0f, text_color);
            else
                DrawText(name, (int)pos.x, (int)pos.y, name_font, text_color);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec((Vector2){(float)mouse_x, (float)mouse_y}, cell)) {
            grid->selected_index = (int)i;
        }
    }
}
