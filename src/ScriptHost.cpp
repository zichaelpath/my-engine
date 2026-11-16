#include "ScriptHost.hpp"

#include "Log.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

ScriptHost::ScriptHost()
{
    m_state = luaL_newstate();
    if (!m_state)
    {
        Log::error("ScriptHost: luaL_newstate failed — scripting is off");
        return;
    }
    luaL_openlibs(m_state);
    Log::info("ScriptHost: Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
              " ready");
}

ScriptHost::~ScriptHost()
{
    if (m_state)
        lua_close(m_state);
}

bool ScriptHost::runString(const std::string& code,
                           const std::string& chunkName)
{
    if (!m_state)
        return false;
    if (luaL_loadbufferx(m_state, code.data(), code.size(),
                         chunkName.c_str(), "t") != LUA_OK)
    {
        Log::error("Lua [" + chunkName + "]: " + lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
        return false;
    }
    if (lua_pcall(m_state, 0, 0, 0) != LUA_OK)
    {
        Log::error("Lua [" + chunkName + "]: " + lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
        return false;
    }
    return true;
}

bool ScriptHost::runFile(const std::string& path)
{
    if (!m_state)
        return false;
    if (luaL_loadfilex(m_state, path.c_str(), "t") != LUA_OK)
    {
        Log::error("Lua [" + path + "]: " + lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
        return false;
    }
    if (lua_pcall(m_state, 0, 0, 0) != LUA_OK)
    {
        Log::error("Lua [" + path + "]: " + lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
        return false;
    }
    Log::info("Lua: ran " + path);
    return true;
}

bool ScriptHost::evalNumber(const std::string& code, double& out)
{
    if (!m_state)
        return false;
    if (luaL_loadbufferx(m_state, code.data(), code.size(), "eval", "t") !=
        LUA_OK)
    {
        Log::error("Lua [eval]: " + std::string(lua_tostring(m_state, -1)));
        lua_pop(m_state, 1);
        return false;
    }
    if (lua_pcall(m_state, 0, 1, 0) != LUA_OK)
    {
        Log::error("Lua [eval]: " + std::string(lua_tostring(m_state, -1)));
        lua_pop(m_state, 1);
        return false;
    }
    if (!lua_isnumber(m_state, -1))
    {
        lua_pop(m_state, 1);
        return false;
    }
    out = lua_tonumber(m_state, -1);
    lua_pop(m_state, 1);
    return true;
}
