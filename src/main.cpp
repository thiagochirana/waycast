#include "ConfigLoader.hpp"
#include "ThemeManager.hpp"
#include "UIManager.hpp"
#include <adwaita.h>
#include <iostream>

static void on_activate(GApplication *app, gpointer user_data) {
    ThemeManager::apply("default");
    ui_manager_start(GTK_APPLICATION(app));
}

int main(int argc, char *argv[]) {
    g_autoptr(AdwApplication) app = adw_application_new(
        "com.waycast.core",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
