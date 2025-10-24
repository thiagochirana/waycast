#pragma once
#include <string>
#include <gtk/gtk.h>
#include <adwaita.h>

class ThemeManager {
public:
    static void apply(const std::string& themeName);
};
