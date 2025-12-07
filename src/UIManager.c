#include "UIManager.h"
#include <adwaita.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    char *exec;
    char *icon;
} App;

static GtkWidget *search_entry = NULL;
static GtkWidget *flow_box = NULL;
static GtkWidget *main_window = NULL;
static GPtrArray *apps = NULL;
static const Config *app_config = NULL;

static void free_app(gpointer data) {
    App *app = (App *)data;
    if (!app)
        return;

    g_free(app->name);
    g_free(app->exec);
    g_free(app->icon);
    g_free(app);
}

static void load_apps(void) {
    if (!apps)
        apps = g_ptr_array_new_with_free_func(free_app);
    else
        g_ptr_array_set_size(apps, 0);

    const char *dirs[] = {
        "/usr/share/applications",
        g_build_filename(g_get_home_dir(), ".local", "share", "applications", NULL)
    };

    for (guint i = 0; i < G_N_ELEMENTS(dirs); i++) {
        const char *dir_path = dirs[i];
        if (!dir_path || !g_file_test(dir_path, G_FILE_TEST_IS_DIR)) {
            if (i == 1)
                g_free((gpointer)dir_path);
            continue;
        }

        GDir *dir = g_dir_open(dir_path, 0, NULL);
        if (!dir) {
            if (i == 1)
                g_free((gpointer)dir_path);
            continue;
        }

        const gchar *entry_name;
        while ((entry_name = g_dir_read_name(dir)) != NULL) {
            if (!g_str_has_suffix(entry_name, ".desktop"))
                continue;

            gchar *full_path = g_build_filename(dir_path, entry_name, NULL);
            GKeyFile *file = g_key_file_new();
            if (g_key_file_load_from_file(file, full_path, G_KEY_FILE_NONE, NULL)) {
                gchar *name = g_key_file_get_string(file, "Desktop Entry", "Name", NULL);
                gchar *exec = g_key_file_get_string(file, "Desktop Entry", "Exec", NULL);
                gchar *icon = g_key_file_get_string(file, "Desktop Entry", "Icon", NULL);

                if (name && exec) {
                    App *app = g_new0(App, 1);
                    app->name = name;
                    app->exec = exec;
                    app->icon = icon;
                    g_ptr_array_add(apps, app);
                } else {
                    g_free(name);
                    g_free(exec);
                    g_free(icon);
                }
            }
            g_key_file_free(file);
            g_free(full_path);
        }

        g_dir_close(dir);
        if (i == 1)
            g_free((gpointer)dir_path);
    }
}

static GtkWidget *create_app_item(App *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(box, "app-item");
    gtk_widget_set_focusable(box, TRUE);
    gtk_widget_set_halign(box, GTK_ALIGN_FILL);
    gtk_widget_set_valign(box, GTK_ALIGN_FILL);

    GtkWidget *image = NULL;
    if (app->icon && *app->icon) {
        if (strchr(app->icon, '/')) {
            if (g_file_test(app->icon, G_FILE_TEST_EXISTS))
                image = gtk_image_new_from_file(app->icon);
            else
                image = gtk_image_new_from_icon_name(app->icon);
        } else {
            image = gtk_image_new_from_icon_name(app->icon);
        }
    }

    if (!image)
        image = gtk_image_new_from_icon_name("application-x-executable");

    gtk_image_set_pixel_size(GTK_IMAGE(image), 80);
    gtk_widget_set_halign(image, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(image, "app-icon");

    GtkWidget *label = gtk_label_new(app->name);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(label, "app-name");
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);

    gtk_box_append(GTK_BOX(box), image);
    gtk_box_append(GTK_BOX(box), label);

    g_object_set_data(G_OBJECT(box), "app-data", app);

    return box;
}

static GtkFlowBoxChild *get_selected_child(void) {
    GList *selected = gtk_flow_box_get_selected_children(GTK_FLOW_BOX(flow_box));
    GtkFlowBoxChild *child = NULL;
    if (selected) {
        child = GTK_FLOW_BOX_CHILD(selected->data);
        g_list_free(selected);
    }
    return child;
}

static void select_first_item(gboolean grab_focus) {
    GtkFlowBoxChild *child = gtk_flow_box_get_child_at_index(GTK_FLOW_BOX(flow_box), 0);
    if (!child)
        return;

    gtk_flow_box_select_child(GTK_FLOW_BOX(flow_box), child);
    if (grab_focus)
        gtk_widget_grab_focus(GTK_WIDGET(child));
}

static void launch_app_from_child(GtkFlowBoxChild *child) {
    if (!child)
        return;

    GtkWidget *content = gtk_flow_box_child_get_child(child);
    if (!content)
        return;

    App *app = g_object_get_data(G_OBJECT(content), "app-data");
    if (!app || !app->exec)
        return;

    g_spawn_command_line_async(app->exec, NULL);
    gtk_window_close(GTK_WINDOW(main_window));
}

static gchar *casefold_text(const gchar *text) {
    if (!text || !*text)
        return NULL;
    return g_utf8_casefold(text, -1);
}

static void filter_apps(GtkEditable *editable, gpointer user_data) {
    (void)user_data;

    const gchar *text = gtk_editable_get_text(editable);
    gtk_flow_box_remove_all(GTK_FLOW_BOX(flow_box));

    if (!text || strlen(text) == 0)
        return;

    gchar *lowered_text = casefold_text(text);
    if (!lowered_text)
        return;

    for (guint i = 0; i < apps->len; i++) {
        App *app = g_ptr_array_index(apps, i);
        if (!app || !app->name)
            continue;

        gchar *lowered_name = casefold_text(app->name);
        if (lowered_name && g_strstr_len(lowered_name, -1, lowered_text)) {
            GtkWidget *item = create_app_item(app);
            gtk_flow_box_append(GTK_FLOW_BOX(flow_box), item);
            GtkWidget *child = gtk_widget_get_last_child(flow_box);
            if (child)
                gtk_widget_set_focusable(child, TRUE);
        }
        g_free(lowered_name);
    }

    g_free(lowered_text);
    select_first_item(FALSE);
}

static void on_flowbox_child_activated(GtkFlowBox *box, GtkFlowBoxChild *child, gpointer user_data) {
    (void)box;
    (void)user_data;
    launch_app_from_child(child);
}

static gboolean on_flowbox_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    (void)controller;
    (void)keycode;
    (void)state;
    (void)user_data;

    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
        GtkFlowBoxChild *child = get_selected_child();
        if (!child)
            child = gtk_flow_box_get_child_at_index(GTK_FLOW_BOX(flow_box), 0);
        if (child) {
            launch_app_from_child(child);
            return TRUE;
        }
        return TRUE;
    }
    return FALSE;
}

static gboolean on_search_entry_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    (void)controller;
    (void)keycode;
    (void)state;
    (void)user_data;

    if (keyval == GDK_KEY_Down || keyval == GDK_KEY_Right) {
        select_first_item(TRUE);
        return TRUE;
    } else if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
        GtkFlowBoxChild *child = gtk_flow_box_get_child_at_index(GTK_FLOW_BOX(flow_box), 0);
        if (child) {
            launch_app_from_child(child);
            return TRUE;
        }
    }
    return FALSE;
}

static GtkWidget *create_main_layout(void) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);

    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search apps...");
    gtk_box_append(GTK_BOX(vbox), search_entry);

    flow_box = gtk_flow_box_new();
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow_box), 16);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow_box), 16);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow_box), 4);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow_box), 1);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow_box), GTK_SELECTION_BROWSE);
    gtk_widget_add_css_class(flow_box, "app-grid");
    gtk_widget_set_hexpand(flow_box, TRUE);
    gtk_widget_set_vexpand(flow_box, TRUE);
    gtk_widget_set_focusable(flow_box, TRUE);

    GtkEventController *flowbox_key_controller = gtk_event_controller_key_new();
    g_signal_connect(flowbox_key_controller, "key-pressed", G_CALLBACK(on_flowbox_key_pressed), NULL);
    gtk_widget_add_controller(flow_box, flowbox_key_controller);

    GtkWidget *scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_widget_add_css_class(scroller, "app-grid-scroller");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), flow_box);

    gtk_box_append(GTK_BOX(vbox), scroller);

    GtkEventController *entry_key_controller = gtk_event_controller_key_new();
    g_signal_connect(entry_key_controller, "key-pressed", G_CALLBACK(on_search_entry_key_pressed), NULL);
    gtk_widget_add_controller(search_entry, entry_key_controller);

    g_signal_connect(search_entry, "changed", G_CALLBACK(filter_apps), NULL);
    g_signal_connect(flow_box, "child-activated", G_CALLBACK(on_flowbox_child_activated), NULL);

    return vbox;
}

static GtkWidget *create_main_window(GtkApplication *app) {
    GtkWidget *win = adw_application_window_new(app);
    main_window = win;

    int width = 600;
    int height = 400;
    if (app_config) {
        width = app_config->window.width;
        height = app_config->window.height;
    }

    gtk_window_set_title(GTK_WINDOW(win), "Waycast");
    gtk_window_set_default_size(GTK_WINDOW(win), width, height);

    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(win), NULL);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(win), TRUE);

    gtk_widget_add_css_class(win, "waycast-overlay");
    return win;
}

void ui_manager_start(GtkApplication *app, const Config *config) {
    app_config = config;
    load_apps();

    GtkWidget *win = create_main_window(app);
    GtkWidget *layout = create_main_layout();

    adw_application_window_set_content(ADW_APPLICATION_WINDOW(win), layout);
    gtk_window_present(GTK_WINDOW(win));
}
