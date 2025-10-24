#include "ConfigLoader.hpp"
#include <fstream>
#include <toml++/toml.h>

ConfigLoader::ConfigLoader(const std::string& path) {
    try {
        auto tbl = toml::parse_file(path);
        theme = tbl["theme"].value_or("default");
    } catch (...) {
        theme = "default";
    }
}

std::string ConfigLoader::getTheme() const {
    return theme;
}
