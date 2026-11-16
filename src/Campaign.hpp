#pragma once

#include <string>
#include <vector>

// A Campaign is a game's level sequence and the player's
// progress through it (Lesson 79): an ordered list of levels
// (project-relative scene paths, exactly as the project card
// lists them), which of them are completed, and which one is
// in play. Pure state and rules — no files, no graphics: hosts
// present it (the Runtime's level select), Game feeds it (a
// touched Goal completes a level), and Lesson 81's save file
// will persist it.
//
// The unlock rule is the genre's standard: level 0 is always
// open; level i opens when level i-1 is completed. The model
// itself is permissive (complete() accepts any index — a host
// may allow bonus or secret completions); hosts gate entry
// with isUnlocked().
struct Campaign
{
    std::vector<std::string> levels;
    std::vector<bool> completed; // parallel to levels
    int current = 0;             // the level in play

    // (Re)build the campaign over a level list: all progress
    // cleared, the first level in play.
    void reset(const std::vector<std::string>& levelList);

    bool isUnlocked(int index) const;
    bool isCompleted(int index) const;
    // Marks a level completed and moves `current` to the next
    // uncompleted level (or leaves it when none remains).
    // False for an out-of-range index.
    bool complete(int index);
    bool completeCurrent();
    // The first uncompleted level's index, or -1 when the
    // campaign is finished.
    int nextLevel() const;
    bool finished() const;
    int levelCount() const { return static_cast<int>(levels.size()); }
};
