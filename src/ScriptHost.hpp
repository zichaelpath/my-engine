#pragma once

#include <string>

// The scripting host (Lesson 66, Arc 5): one embedded Lua state,
// owned and driven through a small C++ face. Scripts are a game's
// own code — written by the game's author, living in the
// project's scripts/ folder — so the host opens Lua's standard
// libraries in full, the way the game's C++ would be trusted.
//
// Errors are values here: a script that fails to compile or run
// logs its message and returns false; it never takes the engine
// down with it. That discipline is what lets Lesson 70 call
// scripts from inside the frame loop safely.
//
// The header forward-declares lua_State so includers do not pull
// in the Lua headers; the binding layer (Lesson 67) includes them
// itself where it needs them.
struct lua_State;

class ScriptHost
{
public:
    ScriptHost();
    ~ScriptHost();
    ScriptHost(const ScriptHost&) = delete;
    ScriptHost& operator=(const ScriptHost&) = delete;

    bool valid() const { return m_state != nullptr; }

    // Runs a chunk of Lua source. chunkName names it in error
    // messages ("string", a file name, ...). Returns false — with
    // the error logged — on any compile or runtime failure.
    bool runString(const std::string& code,
                   const std::string& chunkName = "chunk");

    // Runs a .lua file; a missing file is a false, not a crash.
    bool runFile(const std::string& path);

    // Runs a chunk expected to `return` a number (tests, tools,
    // configuration reads). False when the chunk fails or the
    // value is not numeric.
    bool evalNumber(const std::string& code, double& out);

    lua_State* state() { return m_state; }

private:
    lua_State* m_state = nullptr;
};
