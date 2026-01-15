#include "ui/ui_app_data.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void free_app(gpointer data) {
    App *app = (App *)data;
    if (!app)
        return;

    g_free(app->name);
    g_free(app->exec);
    g_free(app->icon);
    g_free(app);
}

static gchar *casefold_text(const gchar *text) {
    if (!text || !*text)
        return NULL;
    return g_utf8_casefold(text, -1);
}

void ui_app_data_init(UIAppData *data) {
    if (!data)
        return;

    data->apps = g_ptr_array_new_with_free_func(free_app);
    data->filtered_indices = g_array_new(FALSE, FALSE, sizeof(guint));
}

void ui_app_data_free(UIAppData *data) {
    if (!data)
        return;

    if (data->apps)
        g_ptr_array_free(data->apps, TRUE);
    if (data->filtered_indices)
        g_array_free(data->filtered_indices, TRUE);

    data->apps = NULL;
    data->filtered_indices = NULL;
}

void ui_app_data_load(UIAppData *data) {
    if (!data || !data->apps)
        return;

    g_ptr_array_set_size(data->apps, 0);

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
                    g_ptr_array_add(data->apps, app);
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

void ui_app_data_filter(UIAppData *data, const char *query) {
    if (!data || !data->filtered_indices)
        return;

    g_array_set_size(data->filtered_indices, 0);

    if (!query || query[0] == '\0')
        return;

    gchar *lowered_query = casefold_text(query);
    if (!lowered_query)
        return;

    for (guint i = 0; i < data->apps->len; i++) {
        App *app = g_ptr_array_index(data->apps, i);
        if (!app || !app->name)
            continue;

        gchar *lowered_name = casefold_text(app->name);
        if (lowered_name && g_strstr_len(lowered_name, -1, lowered_query)) {
            g_array_append_val(data->filtered_indices, i);
        }
        g_free(lowered_name);
    }

    g_free(lowered_query);
}

guint ui_app_data_filtered_count(const UIAppData *data) {
    if (!data || !data->filtered_indices)
        return 0;
    return data->filtered_indices->len;
}

guint ui_app_data_filtered_index(const UIAppData *data, guint filtered_pos) {
    if (!data || !data->filtered_indices || filtered_pos >= data->filtered_indices->len)
        return 0;
    return g_array_index(data->filtered_indices, guint, filtered_pos);
}

const App *ui_app_data_get(const UIAppData *data, guint index) {
    if (!data || !data->apps || index >= data->apps->len)
        return NULL;
    return g_ptr_array_index(data->apps, index);
}

void ui_app_launch(const App *app) {
    if (!app || !app->exec)
        return;
    g_spawn_command_line_async(app->exec, NULL);
}
