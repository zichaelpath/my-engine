#include "Config.hpp"

#include "Log.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace
{
std::string trim(const std::string& s)
{
    std::size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    std::size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
} // namespace

void Config::setDefault(const std::string& key, const std::string& value)
{
    m_values[key] = value;
}

void Config::set(const std::string& key, const std::string& value)
{
    m_values[key] = value;
}

bool Config::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        Log::info("Config: " + path + " not found, using defaults");
        return false;
    }

    std::string line;
    int lineNumber = 0;
    int loaded = 0;
    while (std::getline(file, line))
    {
        ++lineNumber;
        std::string text = trim(line);
        if (text.empty() || text[0] == '#')
            continue;
        std::size_t eq = text.find('=');
        if (eq == std::string::npos)
        {
            Log::warning("Config: ignoring malformed line " +
                         std::to_string(lineNumber) + ": " + text);
            continue;
        }
        m_values[trim(text.substr(0, eq))] = trim(text.substr(eq + 1));
        ++loaded;
    }
    Log::info("Config: loaded " + std::to_string(loaded) + " values from " + path);
    return true;
}

void Config::save(const std::string& path) const
{
    std::ofstream file(path);
    if (!file)
    {
        Log::error("Config: could not write " + path);
        return;
    }
    file << "# MyEngine configuration (written on first run; edit freely)\n";
    for (const auto& [key, value] : m_values)
        file << key << " = " << value << '\n';
    Log::info("Config: wrote defaults to " + path);
}

int Config::getInt(const std::string& key, int fallback) const
{
    auto it = m_values.find(key);
    if (it == m_values.end())
        return fallback;
    try
    {
        return std::stoi(it->second);
    }
    catch (...)
    {
        return fallback;
    }
}

float Config::getFloat(const std::string& key, float fallback) const
{
    auto it = m_values.find(key);
    if (it == m_values.end())
        return fallback;
    try
    {
        return std::stof(it->second);
    }
    catch (...)
    {
        return fallback;
    }
}

bool Config::getBool(const std::string& key, bool fallback) const
{
    auto it = m_values.find(key);
    if (it == m_values.end())
        return fallback;
    std::string v = toLower(it->second);
    if (v == "true" || v == "1" || v == "yes")
        return true;
    if (v == "false" || v == "0" || v == "no")
        return false;
    return fallback;
}

std::string Config::getString(const std::string& key, const std::string& fallback) const
{
    auto it = m_values.find(key);
    return it == m_values.end() ? fallback : it->second;
}
