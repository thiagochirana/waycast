#include "ConfigLoader.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static void config_load_from_file(Config *config, const char *path);

void config_init(Config *config) {
    if (!config)
        return;

    config->window.width = 600;
    config->window.height = 400;
    config->theme = g_strdup("default");
}

void config_free(Config *config) {
    if (!config)
        return;

    g_clear_pointer(&config->theme, g_free);
}

static gboolean try_get_int(GKeyFile *key_file, const char *group, const char *key, int *out_value) {
    GError *error = NULL;
    gint value = g_key_file_get_integer(key_file, group, key, &error);
    if (error) {
        g_clear_error(&error);
        return FALSE;
    }
    *out_value = (int)value;
    return TRUE;
}

static void config_load_from_file(Config *config, const char *path) {
    GKeyFile *key_file = g_key_file_new();
    GError *error = NULL;

    if (!g_key_file_load_from_file(key_file, path, G_KEY_FILE_NONE, &error)) {
        g_printerr("[Waycast] Failed to load config from %s: %s\n", path, error->message);
        g_error_free(error);
        g_key_file_free(key_file);
        return;
    }

    int width = config->window.width;
    int height = config->window.height;
    if (try_get_int(key_file, "window", "width", &width))
        config->window.width = width;
    if (try_get_int(key_file, "window", "height", &height))
        config->window.height = height;

    gchar *theme = g_key_file_get_string(key_file, "general", "theme", NULL);
    if (theme) {
        g_free(config->theme);
        config->theme = theme;
    }

    g_key_file_free(key_file);
}

void config_load(Config *config, const char *path) {
    if (!config)
        return;

    const gchar *home = g_get_home_dir();
    gchar *user_path = g_build_filename(home, ".config", "waycast", "config.toml", NULL);
    const char *system_path = "/usr/share/waycast/config.toml";

    if (path && *path) {
        config_load_from_file(config, path);
    } else if (g_file_test(user_path, G_FILE_TEST_EXISTS)) {
        config_load_from_file(config, user_path);
    } else if (g_file_test(system_path, G_FILE_TEST_EXISTS)) {
        config_load_from_file(config, system_path);
    } else {
        g_printerr("[Waycast] Config.toml not found or invalid. Using defaults.\n");
    }

    g_free(user_path);
}
