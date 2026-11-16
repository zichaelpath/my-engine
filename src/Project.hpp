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

    // Every scene the project declares, in order (Lesson 78):
    // a card may repeat its scene= line, and the list is the
    // project's level set — the first entry is the start scene
    // (startScene mirrors it). Hosts move between these with
    // Exit triggers; a one-scene project is a list of one.
    std::vector<std::string> scenes;

    // What kind of game this is (Lesson 76): "scene" (the
    // default — a generated world plus an optional start scene,
    // Gold Rush's shape) or "ascent" (a chunk-dealt endless
    // climb, Temple Run's shape, whose chunks live in the file
    // chunksFile names). Additive format-1 keys: a card without
    // them is a scene game, exactly as before.
    std::string mode = "scene";
    std::string chunksFile; // project-relative; ascent mode only

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

// One entry in a single directory (Lesson 62): the file browser's
// data. listDirectory returns directories first, then files, each
// sorted by name, hidden entries skipped — the order every file
// dialog on earth uses, so the browser feels familiar. A missing
// or unreadable directory lists nothing.
struct DirEntry
{
    std::string name;
    std::string fullPath;
    bool isDirectory = false;
    std::uintmax_t size = 0;
};
std::vector<DirEntry> listDirectory(const std::string& path);

// Imports a file into the project (Lesson 63): copies it into
// <root>/assets/ and returns the project-relative path it landed
// at ("assets/hero.png"), or "" on failure. Import COPIES — the
// source stays where it was. Collision-safe: a taken name earns
// a numeric suffix (hero-2.png, hero-3.png, ...) — importing
// never overwrites an existing asset.
std::string importAsset(const Project& project,
                        const std::string& sourcePath);
