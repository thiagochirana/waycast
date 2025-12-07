#include "ThemeManager.h"
#include <gtk/gtk.h>
#include <stdlib.h>

void theme_manager_apply(const char *theme_name) {
    const char *name = (theme_name && *theme_name) ? theme_name : "default";

    const gchar *home = g_get_home_dir();
    gchar *user_path = g_build_filename(home, ".config", "waycast", "themes", name, "style.css", NULL);
    gchar *path = NULL;

    if (g_file_test(user_path, G_FILE_TEST_EXISTS)) {
        path = g_strdup(user_path);
    } else {
        gchar *system_path = g_build_filename("/usr/share/waycast/themes", name, "style.css", NULL);
        if (g_file_test(system_path, G_FILE_TEST_EXISTS))
            path = system_path;
        else
            g_free(system_path);
    }

    g_free(user_path);

    if (!path)
        return;

    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, path);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_free(path);
    g_object_unref(css);
}
