#include "UIManager.hpp"
#include <gtk/gtk.h>
#include <adwaita.h>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <vector>
#include <toml++/toml.hpp>

struct Config {
    int width = 600;
    int height = 400;
};

static GtkWidget *search_entry;
static GtkWidget *list_box;
static std::vector<std::pair<std::string, std::string>> apps;
static Config config;

static void load_config() {
    try {
        auto cfg = toml::parse_file("/usr/share/waycast/config.toml");
        if (cfg["window"]["width"].is_integer())
            config.width = *cfg["window"]["width"].value<int>();
        if (cfg["window"]["height"].is_integer())
            config.height = *cfg["window"]["height"].value<int>();
    } catch (const std::exception &e) {
        g_printerr("Config.toml não encontrado ou inválido. Usando padrão.\n");
    }
}


static void load_apps() {
    apps.clear();
    std::vector<std::string> dirs = {
        "/usr/share/applications",
        std::string(std::getenv("HOME")) + "/.local/share/applications"
    };

    for (auto &dir : dirs) {
        if (!std::filesystem::exists(dir)) continue;
        for (auto &entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".desktop") {
                GKeyFile *file = g_key_file_new();
                if (g_key_file_load_from_file(file, entry.path().c_str(), G_KEY_FILE_NONE, NULL)) {
                    gchar *name = g_key_file_get_string(file, "Desktop Entry", "Name", NULL);
                    gchar *exec = g_key_file_get_string(file, "Desktop Entry", "Exec", NULL);
                    if (name && exec)
                        apps.emplace_back(name, exec);
                    g_free(name);
                    g_free(exec);
                }
                g_key_file_free(file);
            }
        }
    }

    for (auto &[name, exec] : apps) {
        GtkWidget *row = gtk_label_new(name.c_str());
        gtk_widget_set_halign(row, GTK_ALIGN_START);
        gtk_list_box_append(GTK_LIST_BOX(list_box), row);
    }
}

static void filter_apps(GtkEditable *editable, gpointer) {
    const gchar *text = gtk_editable_get_text(editable);
    for (GtkWidget *row = gtk_widget_get_first_child(list_box);
         row != NULL;
         row = gtk_widget_get_next_sibling(row)) {

        GtkWidget *child = gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(row));
        if (!GTK_IS_LABEL(child)) continue;
        const gchar *label_text = gtk_label_get_text(GTK_LABEL(child));
        if (!label_text) continue;

        gboolean visible = (strlen(text) == 0) ||
                           (g_strrstr(g_utf8_strdown(label_text, -1),
                                      g_utf8_strdown(text, -1)) != NULL);
        gtk_widget_set_visible(row, visible);
    }
}

static void launch_app(GtkListBox *, GtkListBoxRow *row, gpointer) {
    int index = gtk_list_box_row_get_index(row);
    if (index >= 0 && index < (int)apps.size()) {
        std::string cmd = apps[index].second + " &";
        std::system(cmd.c_str());
    }
}

static GtkWidget* create_main_layout() {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);

    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search apps...");
    gtk_box_append(GTK_BOX(vbox), search_entry);

    list_box = gtk_list_box_new();
    gtk_box_append(GTK_BOX(vbox), list_box);

    g_signal_connect(search_entry, "changed", G_CALLBACK(filter_apps), NULL);
    g_signal_connect(list_box, "row-activated", G_CALLBACK(launch_app), NULL);

    return vbox;
}

static GtkWidget* create_main_window(GtkApplication *app) {
    GtkWidget *win = adw_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Waycast");
    gtk_window_set_default_size(GTK_WINDOW(win), config.width, config.height);

    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), NULL);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(win), TRUE);

    gtk_widget_add_css_class(win, "waycast-overlay");
    return win;
}

void ui_manager_start(GtkApplication *app) {
    load_config();
    GtkWidget *win = create_main_window(app);
    GtkWidget *layout = create_main_layout();

    adw_application_window_set_content(ADW_APPLICATION_WINDOW(win), layout);
    load_apps();

    gtk_window_present(GTK_WINDOW(win));
}
