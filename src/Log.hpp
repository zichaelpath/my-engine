#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class LogLevel
{
    Info,
    Warning,
    Error
};

struct LogEntry
{
    LogLevel level;
    std::string message;
};

// The engine's voice. Any system in GameLib can log; hosts decide where
// the lines go — the editor shows them in its Console panel, and every
// line is also mirrored to stdout so the runtime (and any terminal)
// sees them too. Entries are capped: when the buffer is full, the
// oldest lines are dropped.
class Log
{
public:
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

    static const std::vector<LogEntry>& entries();
    static void clear();

private:
    static void add(LogLevel level, const std::string& message);

    static constexpr std::size_t kMaxEntries = 500;
    static inline std::vector<LogEntry> s_entries;
};
