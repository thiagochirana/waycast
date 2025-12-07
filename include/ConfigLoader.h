#pragma once
#include <stddef.h>

typedef struct {
    int width;
    int height;
} WindowConfig;

typedef struct {
    WindowConfig window;
    char *theme;
} Config;

void config_init(Config *config);
void config_load(Config *config, const char *path);
void config_free(Config *config);
