#include "ConfigLoader.h"
#include <glib.h>
#include <stdio.h>

void config_init(Config *config) {
    if (!config)
        return;

    config->window.width = 1000;
    config->window.height = 600;
    config->theme = g_strdup("default");
}

void config_free(Config *config) {
    if (!config)
        return;

    g_clear_pointer(&config->theme, g_free);
}

void config_load(Config *config, const char *path) {
    (void)config;
    (void)path;
}
