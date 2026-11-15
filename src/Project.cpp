#include "Project.hpp"

#include "Log.hpp"

#include <fstream>
#include <sstream>

namespace
{

std::string trimmed(const std::string& s)
{
    std::size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    std::size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

} // namespace

bool loadProject(const std::string& path, Project& out)
{
    std::ifstream file(path);
    if (!file)
        return false; // no project file is a normal state, not an error

    Project parsed;
    bool sawFormat = false;
    std::string line;
    while (std::getline(file, line))
    {
        std::string t = trimmed(line);
        if (t.empty() || t[0] == '#')
            continue;
        std::size_t eq = t.find('=');
        if (eq == std::string::npos)
        {
            Log::error("Project file " + path + ": line without '=' ignored: " + t);
            continue;
        }
        std::string key = trimmed(t.substr(0, eq));
        std::string value = trimmed(t.substr(eq + 1));
        if (key == "format")
        {
            parsed.formatVersion = std::stoi(value);
            sawFormat = true;
        }
        else if (key == "name")
        {
            parsed.name = value;
        }
        else if (key == "seed")
        {
            parsed.seed = static_cast<std::uint32_t>(std::stoul(value));
        }
        else if (key == "scene")
        {
            parsed.startScene = value;
        }
        else
        {
            Log::warning("Project file " + path + ": unknown key '" + key + "' ignored");
        }
    }

    if (!sawFormat)
    {
        Log::error("Project file " + path + " has no format= line; not loaded");
        return false;
    }
    if (parsed.formatVersion != 1)
    {
        Log::error("Project file " + path + " is format " +
                   std::to_string(parsed.formatVersion) +
                   "; this engine reads format 1. Not loaded.");
        return false;
    }

    out = parsed;
    Log::info("Project loaded: " + out.name + " (format " +
              std::to_string(out.formatVersion) + ", seed " +
              std::to_string(out.seed) + ", scene " +
              (out.startScene.empty() ? std::string("(none)") : out.startScene) + ")");
    return true;
}
