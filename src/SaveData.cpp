#include "SaveData.hpp"

#include "Log.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>

namespace
{
std::string savePath(const std::string& projectRoot)
{
    if (projectRoot.empty() || projectRoot == ".")
        return "save.dat";
    std::string root = projectRoot;
    while (root.size() > 1 && root.back() == '/')
        root.pop_back();
    return root + "/save.dat";
}

std::string seconds(float value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f",
                  static_cast<double>(value));
    return buf;
}
} // namespace

float SaveData::bestTime(std::uint32_t seed) const
{
    const auto it = bestTimes.find(seed);
    return it == bestTimes.end() ? 0.f : it->second;
}

bool SaveData::recordTime(std::uint32_t seed, float value)
{
    const float best = bestTime(seed);
    if (best > 0.f && value >= best)
        return false;
    bestTimes[seed] = value;
    return true;
}

bool SaveData::recordHeight(int height)
{
    if (height <= templeBestHeight)
        return false;
    templeBestHeight = height;
    return true;
}

std::string SaveData::setting(const std::string& key,
                              const std::string& fallback) const
{
    const auto it = settings.find(key);
    return it == settings.end() ? fallback : it->second;
}

void SaveData::setSetting(const std::string& key,
                          const std::string& value)
{
    settings[key] = value;
}

bool loadSave(const std::string& projectRoot, SaveData& out)
{
    const std::string path = savePath(projectRoot);
    std::ifstream in(path);
    if (!in)
        return false; // no save yet: the ordinary first run

    SaveData parsed;
    std::string line;
    bool sawFormat = false;
    while (std::getline(in, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        try
        {
            if (key == "format")
            {
                parsed.formatVersion = std::stoi(value);
                sawFormat = true;
            }
            else if (key == "campaignLevels")
            {
                const int n = std::stoi(value);
                if (n > 0)
                {
                    parsed.hasCampaign = true;
                    parsed.campaign.levels.assign(
                        static_cast<std::size_t>(n), "");
                    parsed.campaign.completed.assign(
                        static_cast<std::size_t>(n), false);
                }
            }
            else if (key.rfind("campaignLevel.", 0) == 0 &&
                     parsed.hasCampaign)
            {
                const int i = std::stoi(key.substr(14));
                if (i >= 0 &&
                    i < parsed.campaign.levelCount())
                    parsed.campaign.levels[static_cast<std::size_t>(i)] =
                        value;
            }
            else if (key == "campaignDone" && parsed.hasCampaign)
            {
                std::stringstream ss(value);
                std::string flag;
                int i = 0;
                while (std::getline(ss, flag, ','))
                {
                    if (i < parsed.campaign.levelCount())
                        parsed.campaign.completed[static_cast<std::size_t>(i)] =
                            flag == "1";
                    ++i;
                }
            }
            else if (key == "campaignCurrent" && parsed.hasCampaign)
            {
                parsed.campaign.current = std::stoi(value);
            }
            else if (key.rfind("bestTime.", 0) == 0)
            {
                parsed.bestTimes[static_cast<std::uint32_t>(
                    std::stoul(key.substr(9)))] = std::stof(value);
            }
            else if (key == "templeBest")
            {
                parsed.templeBestHeight = std::stoi(value);
            }
            else if (key.rfind("setting.", 0) == 0)
            {
                parsed.settings[key.substr(8)] = value;
            }
            // Unknown keys are ignored: a newer save read by an
            // older build keeps what it understands.
        }
        catch (const std::exception&)
        {
            Log::warning("Save line skipped (unparseable): " + line);
        }
    }
    if (!sawFormat || parsed.formatVersion != 1)
    {
        Log::error("Save file refused (format " +
                   std::to_string(parsed.formatVersion) +
                   ", expected 1): " + path);
        return false;
    }
    out = parsed;
    Log::info("Save loaded from " + path);
    return true;
}

bool writeSave(const std::string& projectRoot, const SaveData& save)
{
    const std::string path = savePath(projectRoot);
    std::ofstream file(path);
    if (!file)
    {
        Log::error("Could not write save file: " + path);
        return false;
    }
    file << "# MyEngine save file - format 1\n"
         << "# Player progress for this project: campaign state,\n"
         << "# best times and heights, per-player settings.\n"
         << "format=1\n";
    if (save.hasCampaign)
    {
        file << "campaignLevels=" << save.campaign.levelCount()
             << "\n";
        for (int i = 0; i < save.campaign.levelCount(); ++i)
            file << "campaignLevel." << i << "="
                 << save.campaign.levels[static_cast<std::size_t>(i)]
                 << "\n";
        file << "campaignCurrent=" << save.campaign.current << "\n"
             << "campaignDone=";
        for (int i = 0; i < save.campaign.levelCount(); ++i)
            file << (i ? "," : "")
                 << (save.campaign.completed[static_cast<std::size_t>(i)]
                         ? "1"
                         : "0");
        file << "\n";
    }
    for (const auto& [seed, time] : save.bestTimes)
        file << "bestTime." << seed << "=" << seconds(time) << "\n";
    if (save.templeBestHeight > 0)
        file << "templeBest=" << save.templeBestHeight << "\n";
    for (const auto& [key, value] : save.settings)
        file << "setting." << key << "=" << value << "\n";
    Log::info("Save written to " + path);
    return true;
}
