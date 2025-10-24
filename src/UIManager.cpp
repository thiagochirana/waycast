#include "UIManager.hpp"
#include <gtk/gtk.h>
#include <adwaita.h>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <vector>

static GtkWidget *search_entry;
static GtkWidget *list_box;
static std::vector<std::pair<std::string, std::string>> apps;

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

static void on_search_changed(GtkEditable *editable, gpointer user_data) {
    const gchar *text = gtk_editable_get_text(editable);
    for (GtkWidget *row = gtk_widget_get_first_child(list_box);
         row != NULL;
         row = gtk_widget_get_next_sibling(row)) {

        const gchar *label_text = gtk_label_get_text(GTK_LABEL(row));
        gboolean visible = (strlen(text) == 0) ||
                           (g_strrstr(g_utf8_strdown(label_text, -1),
                                      g_utf8_strdown(text, -1)) != NULL);
        gtk_widget_set_visible(row, visible);
    }
}

static void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    int index = gtk_list_box_row_get_index(row);
    if (index >= 0 && index < (int)apps.size()) {
        std::string cmd = apps[index].second + " &";
        std::system(cmd.c_str());
    }
}

void ui_manager_start(GtkApplication *app) {
    GtkWidget *win = adw_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Waycast");
    gtk_window_set_default_size(GTK_WINDOW(win), 500, 600);

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

    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), NULL);
    g_signal_connect(list_box, "row-activated", G_CALLBACK(on_row_activated), NULL);

    adw_application_window_set_content(ADW_APPLICATION_WINDOW(win), vbox);
    load_apps();
    gtk_window_present(GTK_WINDOW(win));
}
