#pragma once
#include <string>
#include <map>

class ConfigLoader {
public:
    ConfigLoader(const std::string& path);
    std::string getTheme() const;

private:
    std::string theme;
};
