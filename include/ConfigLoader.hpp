#pragma once
#include <string>
#include <toml++/toml.h>

struct WindowConfig {
    int width = 600;
    int height = 400;
};

struct Config {
    WindowConfig window;
    std::string theme = "default";
};

class ConfigLoader {
public:
    explicit ConfigLoader(const std::string &path = "");
    const Config &get() const;

private:
    Config config;
    void loadFromFile(const std::string &path);
};
