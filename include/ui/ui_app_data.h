#pragma once
#include <glib.h>

typedef struct {
    char *name;
    char *exec;
    char *icon;
} App;

typedef struct {
    GPtrArray *apps;
    GArray *filtered_indices;
} UIAppData;

void ui_app_data_init(UIAppData *data);
void ui_app_data_free(UIAppData *data);
void ui_app_data_load(UIAppData *data);
void ui_app_data_filter(UIAppData *data, const char *query);

guint ui_app_data_filtered_count(const UIAppData *data);
guint ui_app_data_filtered_index(const UIAppData *data, guint filtered_pos);
const App *ui_app_data_get(const UIAppData *data, guint index);

void ui_app_launch(const App *app);
