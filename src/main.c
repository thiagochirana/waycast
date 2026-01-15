#include "ConfigLoader.h"
#include "UIManager.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    Config config;
    config_init(&config);
    config_load(&config, NULL);

    (void)argc;
    (void)argv;

    ui_manager_start(&config);
    config_free(&config);
    return 0;
}
