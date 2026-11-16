#pragma once

#include "Campaign.hpp"
#include <cstdint>
#include <map>
#include <string>

// A project's save file (Lesson 81): save.dat in the project
// folder, holding what belongs to the *player* rather than the
// machine (engine.ini) or the game (project.myengine):
//
//   - campaign progress (the Campaign model, levels included
//     so a host can verify the save still matches its game),
//   - Gold Rush best times, per world seed,
//   - Temple Run's best height,
//   - per-player settings as key/value strings (the runtime's
//     camera-shake preference is the first).
//
// The format is the engine's text family, versioned like the
// others: a file whose format= is not 1 is refused whole.
struct SaveData
{
    int formatVersion = 0; // 0 until a file loads successfully

    bool hasCampaign = false;
    Campaign campaign;

    std::map<std::uint32_t, float> bestTimes; // seed -> seconds
    int templeBestHeight = 0;                 // 0 = no climb yet

    std::map<std::string, std::string> settings;

    // The best recorded win time for a seed, or 0 when none.
    float bestTime(std::uint32_t seed) const;
    // Records a win time / a climb height; true when it is a
    // new best (the caller then persists and celebrates).
    bool recordTime(std::uint32_t seed, float seconds);
    bool recordHeight(int height);

    std::string setting(const std::string& key,
                        const std::string& fallback) const;
    void setSetting(const std::string& key, const std::string& value);
};

// save.dat inside the given project root. Loading a missing or
// refused file returns false and leaves `out` untouched.
bool loadSave(const std::string& projectRoot, SaveData& out);
bool writeSave(const std::string& projectRoot, const SaveData& save);
