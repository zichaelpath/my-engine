#include "Project.hpp"

#include "Log.hpp"

#include <filesystem>
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

namespace
{

// A root is stored without a trailing slash (except the
// filesystem root itself), so projectPath can always join with
// exactly one '/'.
std::string normalizedRoot(const std::string& folder)
{
    std::string root = folder;
    while (root.size() > 1 && root.back() == '/')
        root.pop_back();
    return root;
}

bool writeProjectFile(const std::string& path, const Project& project)
{
    std::ofstream file(path);
    if (!file)
        return false;
    file << "# MyEngine project file — format 1\n"
         << "# The game's identity card: hosts read this first. Per-machine\n"
         << "# settings (window size, volume, speed) live in engine.ini "
            "instead.\n"
         << "format=1\n"
         << "name=" << project.name << "\n"
         << "seed=" << project.seed << "\n"
         << "scene=" << project.startScene << "\n";
    return static_cast<bool>(file);
}

} // namespace

std::string projectPath(const Project& project, const std::string& relative)
{
    if (project.root.empty() || relative.empty())
        return relative;
    if (relative.front() == '/') // already absolute
        return relative;
    return project.root + "/" + relative;
}

bool loadProjectFolder(const std::string& folder, Project& out)
{
    Project loaded;
    if (!loadProject(normalizedRoot(folder) + "/project.myengine", loaded))
        return false;
    loaded.root = normalizedRoot(folder);
    out = loaded;
    return true;
}

bool createProjectFolder(const std::string& folder, const std::string& name,
                         Project& out)
{
    namespace fs = std::filesystem;
    const std::string root = normalizedRoot(folder);
    std::error_code ec;
    if (fs::exists(root + "/project.myengine"))
    {
        Log::error("createProjectFolder: a project already lives at " + root +
                   " — open it instead of creating over it");
        return false;
    }
    for (const char* sub : {"", "/assets", "/scenes", "/scripts"})
    {
        if (!fs::create_directories(root + sub, ec) && ec)
        {
            Log::error("createProjectFolder: cannot create " + root + sub +
                       ": " + ec.message());
            return false;
        }
    }
    Project created;
    created.name = name;
    created.formatVersion = 1;
    created.root = root;
    if (!writeProjectFile(root + "/project.myengine", created))
    {
        Log::error("createProjectFolder: cannot write " + root +
                   "/project.myengine");
        return false;
    }
    out = created;
    Log::info("Project created: " + name + " at " + root +
              " (assets/, scenes/, scripts/)");
    return true;
}

bool saveProject(const Project& project)
{
    if (project.root.empty())
    {
        Log::error("saveProject: the project has no root folder");
        return false;
    }
    if (!writeProjectFile(project.root + "/project.myengine", project))
    {
        Log::error("saveProject: cannot write " + project.root +
                   "/project.myengine");
        return false;
    }
    Log::info("Project saved: " + project.name + " at " + project.root);
    return true;
}
