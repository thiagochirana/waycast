#include "ThemeManager.hpp"
#include <gtk/gtk.h>
#include <filesystem>

void ThemeManager::apply(const std::string& themeName) {
    std::string path = std::string(std::getenv("HOME")) + "/.config/waycast/themes/" + themeName + "/style.css";
    if (!std::filesystem::exists(path))
        path = "/usr/share/waycast/themes/" + themeName + "/style.css";

    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, path.c_str());
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}
