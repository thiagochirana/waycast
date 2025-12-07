#include "ConfigLoader.h"
#include "ThemeManager.h"
#include "UIManager.h"
#include <adwaita.h>
#include <gtk/gtk.h>
#include <stdlib.h>

static void on_activate(GApplication *app, gpointer user_data) {
    Config *config = (Config *)user_data;
    const char *theme = (config && config->theme) ? config->theme : "default";

    theme_manager_apply(theme);
    ui_manager_start(GTK_APPLICATION(app), config);
}

int main(int argc, char *argv[]) {
    Config config;
    config_init(&config);
    config_load(&config, NULL);

    AdwApplication *app = adw_application_new(
        "com.waycast.core",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &config);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    config_free(&config);
    return status;
}
