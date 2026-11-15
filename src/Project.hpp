#pragma once

#include <cstdint>
#include <string>

// A Project is the game's identity card (Lesson 41): one small
// text file, project.myengine, that says what game this is — its
// name, its world seed, and the scene it starts from. Hosts
// (Runtime, Editor) load it before anything else; engine.ini stays
// the per-machine settings file (window, volume, speed), while the
// project file travels with the game and wins on the things it
// defines. The format is the family the engine already speaks:
// key=value lines, '#' comments, unknown keys warned and ignored.
struct Project
{
    std::string name = "Untitled";
    int formatVersion = 0; // 0 until a file loads successfully
    std::uint32_t seed = 12345;
    std::string startScene; // empty = generate the world, load nothing
};

// Loads a project file into `out`. Returns false — leaving `out`
// untouched — when the file is missing, unreadable, malformed, or
// written for a different format version: a project from a newer
// engine must fail loudly here, never half-load.
bool loadProject(const std::string& path, Project& out);
