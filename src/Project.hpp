#pragma once

#include <cstdint>
#include <string>
#include <vector>

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

    // The folder the project lives in (Lesson 60): a project is a
    // PLACE — a directory holding project.myengine plus the
    // standard subfolders (assets/, scenes/, scripts/). Empty
    // until the project is loaded or created through the folder
    // API below; paths inside the project are relative to here.
    std::string root;
};

// Loads a project file into `out`. Returns false — leaving `out`
// untouched — when the file is missing, unreadable, malformed, or
// written for a different format version: a project from a newer
// engine must fail loudly here, never half-load.
bool loadProject(const std::string& path, Project& out);

// The folder API (Lesson 60). loadProjectFolder reads
// <folder>/project.myengine and records the folder as the root.
// createProjectFolder makes a new project place: the folder, its
// standard subfolders, and a fresh project.myengine — refusing
// (false) when a project already lives there, because creating
// over one is a different verb than opening it. saveProject
// writes the struct back to <root>/project.myengine.
// projectPath resolves a project-relative path against the root
// (an empty root leaves the path as-is: today's behaviour).
bool loadProjectFolder(const std::string& folder, Project& out);
bool createProjectFolder(const std::string& folder, const std::string& name,
                         Project& out);
bool saveProject(const Project& project);
std::string projectPath(const Project& project, const std::string& relative);

// One entry in the project's tree (Lesson 61): its path relative
// to the root ('/'-joined, e.g. "assets/hero.png"), whether it is
// a directory, and — for files — its size in bytes.
struct ProjectEntry
{
    std::string relativePath;
    bool isDirectory = false;
    std::uintmax_t size = 0;
};

// Lists everything under the project root, recursively, sorted
// by relative path. Hidden entries (a leading '.') are skipped:
// .gitkeep and its kin are plumbing, not project content. An
// empty root (or a missing one) lists nothing.
std::vector<ProjectEntry> listProjectFiles(const Project& project);
