#include "Log.hpp"

#include <iostream>

void Log::info(const std::string& message)
{
    add(LogLevel::Info, message);
}

void Log::warning(const std::string& message)
{
    add(LogLevel::Warning, message);
}

void Log::error(const std::string& message)
{
    add(LogLevel::Error, message);
}

const std::vector<LogEntry>& Log::entries()
{
    return s_entries;
}

void Log::clear()
{
    s_entries.clear();
}

void Log::add(LogLevel level, const std::string& message)
{
    s_entries.push_back({level, message});
    if (s_entries.size() > kMaxEntries)
        s_entries.erase(s_entries.begin());

    const char* tag = level == LogLevel::Warning ? "[warning] "
                      : level == LogLevel::Error ? "[error]   "
                                                 : "[info]    ";
    std::cout << tag << message << '\n';
}
