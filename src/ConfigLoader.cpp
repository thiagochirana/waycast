#include "ConfigLoader.hpp"
#include <filesystem>
#include <iostream>

// priority load order:
// 1. ~/.config/waycast/config.toml
// 2. /usr/share/waycast/config.toml

ConfigLoader::ConfigLoader(const std::string &path) {
    std::string userPath = std::string(std::getenv("HOME")) + "/.config/waycast/config.toml";
    std::string systemPath = "/usr/share/waycast/config.toml";

    if (!path.empty())
        loadFromFile(path);
    else if (std::filesystem::exists(userPath))
        loadFromFile(userPath);
    else if (std::filesystem::exists(systemPath))
        loadFromFile(systemPath);
    else
        std::cerr << "[Waycast] Config.toml not found or invalid. Using defaults." << std::endl;
}

void ConfigLoader::loadFromFile(const std::string &path) {
    try {
        auto tbl = toml::parse_file(path);

        if (tbl["window"]["width"].is_integer())
            config.window.width = *tbl["window"]["width"].value<int>();

        if (tbl["window"]["height"].is_integer())
            config.window.height = *tbl["window"]["height"].value<int>();

        if (tbl["general"]["theme"].is_string())
            config.theme = *tbl["general"]["theme"].value<std::string>();

    } catch (const std::exception &e) {
        std::cerr << "[Waycast] Error loading config: " << e.what() << std::endl;
    }
}

const Config &ConfigLoader::get() const {
    return config;
}
