#include "Campaign.hpp"

#include "Log.hpp"

void Campaign::reset(const std::vector<std::string>& levelList)
{
    levels = levelList;
    completed.assign(levels.size(), false);
    current = 0;
}

bool Campaign::isUnlocked(int index) const
{
    if (index < 0 || index >= levelCount())
        return false;
    return index == 0 || completed[static_cast<std::size_t>(index - 1)];
}

bool Campaign::isCompleted(int index) const
{
    if (index < 0 || index >= levelCount())
        return false;
    return completed[static_cast<std::size_t>(index)];
}

int Campaign::nextLevel() const
{
    for (int i = 0; i < levelCount(); ++i)
        if (!completed[static_cast<std::size_t>(i)])
            return i;
    return -1;
}

bool Campaign::finished() const
{
    return levelCount() > 0 && nextLevel() == -1;
}

bool Campaign::complete(int index)
{
    if (index < 0 || index >= levelCount())
        return false;
    completed[static_cast<std::size_t>(index)] = true;
    Log::info("Campaign: level " + std::to_string(index + 1) + " of " +
              std::to_string(levelCount()) + " completed (" +
              levels[static_cast<std::size_t>(index)] + ")");
    const int next = nextLevel();
    if (next >= 0)
        current = next;
    return true;
}

bool Campaign::completeCurrent()
{
    return complete(current);
}
