#pragma once

#include <map>
#include <string>

// A tiny key=value config (engine.ini). Hosts register their defaults,
// load a file over them, and read typed values back with fallbacks.
//
// A missing file is not an error: the hosts write the defaults out so
// there is a file to edit. Malformed lines are logged as warnings and
// skipped. A config typo must never crash the engine.
class Config
{
public:
    void setDefault(const std::string& key, const std::string& value);

    // Sets a value in memory (a host recording a user's choice,
    // Lesson 84); call save() to persist it.
    void set(const std::string& key, const std::string& value);

    // Returns true if the file existed and was read.
    bool load(const std::string& path);

    // Writes every known key (defaults merged with loaded values).
    void save(const std::string& path) const;

    int getInt(const std::string& key, int fallback) const;
    float getFloat(const std::string& key, float fallback) const;
    bool getBool(const std::string& key, bool fallback) const;
    std::string getString(const std::string& key, const std::string& fallback) const;

private:
    std::map<std::string, std::string> m_values;
};
