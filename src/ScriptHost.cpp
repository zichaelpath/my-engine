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

// --- The bridge (Lesson 67) -------------------------------------------
// The hand-bound API v1. Every binding is a plain C function; the
// Game travels as a lightuserdata upvalue, so there is exactly one
// binding set per install and no hidden global state on the C++
// side. Entities resolve by name (first match) — names are how a
// scene's author already thinks about them.

#include "Game.hpp"

namespace
{

Game* gameOf(lua_State* L)
{
    return static_cast<Game*>(lua_touserdata(L, lua_upvalueindex(1)));
}

// The entity a state table belongs to: its __name field, resolved
// against the game's entity list.
Entity* entityOf(lua_State* L, Game* game, int stateIndex)
{
    if (!game || !lua_istable(L, stateIndex))
        return nullptr;
    lua_getfield(L, stateIndex, "__name");
    const char* name = lua_tostring(L, -1);
    Entity* found = nullptr;
    if (name)
    {
        for (auto& e : game->entities())
            if (e.name == name)
            {
                found = &e;
                break;
            }
    }
    lua_pop(L, 1);
    return found;
}

int apiGetPosition(lua_State* L)
{
    Entity* e = entityOf(L, gameOf(L), 1);
    if (!e)
        return 0;
    lua_pushnumber(L, e->position.x);
    lua_pushnumber(L, e->position.y);
    return 2;
}

int apiSetPosition(lua_State* L)
{
    if (Entity* e = entityOf(L, gameOf(L), 1))
    {
        e->position.x = static_cast<float>(luaL_checknumber(L, 2));
        e->position.y = static_cast<float>(luaL_checknumber(L, 3));
    }
    return 0;
}

int apiMove(lua_State* L)
{
    if (Entity* e = entityOf(L, gameOf(L), 1))
    {
        e->position.x += static_cast<float>(luaL_checknumber(L, 2));
        e->position.y += static_cast<float>(luaL_checknumber(L, 3));
    }
    return 0;
}

int apiGetVelocity(lua_State* L)
{
    Entity* e = entityOf(L, gameOf(L), 1);
    if (!e)
        return 0;
    lua_pushnumber(L, e->velocity.x);
    lua_pushnumber(L, e->velocity.y);
    return 2;
}

int apiSetVelocity(lua_State* L)
{
    if (Entity* e = entityOf(L, gameOf(L), 1))
    {
        e->velocity.x = static_cast<float>(luaL_checknumber(L, 2));
        e->velocity.y = static_cast<float>(luaL_checknumber(L, 3));
    }
    return 0;
}

int apiIsGrounded(lua_State* L)
{
    Entity* e = entityOf(L, gameOf(L), 1);
    lua_pushboolean(L, e && e->grounded);
    return 1;
}

int apiLog(lua_State* L)
{
    const char* message = luaL_tolstring(L, 1, nullptr);
    Log::info(std::string("[script] ") + (message ? message : ""));
    lua_pop(L, 1);
    return 0;
}

int apiTime(lua_State* L)
{
    Game* game = gameOf(L);
    lua_pushnumber(L, game ? game->time : 0.f);
    return 1;
}

} // namespace

void ScriptHost::installEntityApi(Game& game)
{
    if (!m_state)
        return;
    lua_State* L = m_state;
    lua_newtable(L);
    auto bind = [&](const char* name, lua_CFunction fn) {
        lua_pushlightuserdata(L, &game);
        lua_pushcclosure(L, fn, 1);
        lua_setfield(L, -2, name);
    };
    bind("getPosition", apiGetPosition);
    bind("setPosition", apiSetPosition);
    bind("move", apiMove);
    bind("getVelocity", apiGetVelocity);
    bind("setVelocity", apiSetVelocity);
    bind("isGrounded", apiIsGrounded);
    lua_setglobal(L, "entity");

    lua_pushlightuserdata(L, &game);
    lua_pushcclosure(L, apiLog, 1);
    lua_setglobal(L, "log");
    lua_pushlightuserdata(L, &game);
    lua_pushcclosure(L, apiTime, 1);
    lua_setglobal(L, "time");
    Log::info("ScriptHost: entity API v1 installed");
}

bool ScriptHost::loadModuleFromStack(int& moduleId)
{
    // The chunk is on top of the stack; run it and keep what it
    // returns — a module must return its table of functions.
    if (lua_pcall(m_state, 0, 1, 0) != LUA_OK)
    {
        Log::error("Lua [module]: " + std::string(lua_tostring(m_state, -1)));
        lua_pop(m_state, 1);
        return false;
    }
    if (!lua_istable(m_state, -1))
    {
        Log::error("Lua [module]: a script module must return a table");
        lua_pop(m_state, 1);
        return false;
    }
    const int ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
    moduleId = static_cast<int>(m_moduleRefs.size());
    m_moduleRefs.push_back(ref);
    return true;
}

bool ScriptHost::loadModule(const std::string& path, int& moduleId)
{
    if (!m_state)
        return false;
    if (luaL_loadfilex(m_state, path.c_str(), "t") != LUA_OK)
    {
        Log::error("Lua [" + path + "]: " + lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
        return false;
    }
    return loadModuleFromStack(moduleId);
}

bool ScriptHost::loadModuleFromString(const std::string& code,
                                      const std::string& chunkName,
                                      int& moduleId)
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
    return loadModuleFromStack(moduleId);
}

bool ScriptHost::createInstance(int moduleId,
                                const std::string& entityName,
                                int& instanceId)
{
    if (!m_state || moduleId < 0 ||
        moduleId >= static_cast<int>(m_moduleRefs.size()))
        return false;
    lua_newtable(m_state);
    lua_pushstring(m_state, entityName.c_str());
    lua_setfield(m_state, -2, "__name");
    Instance instance;
    instance.moduleRef = m_moduleRefs[moduleId];
    instance.stateRef = luaL_ref(m_state, LUA_REGISTRYINDEX);
    instance.entityName = entityName;
    instanceId = static_cast<int>(m_instances.size());
    m_instances.push_back(instance);
    return true;
}

bool ScriptHost::callUpdate(int instanceId, float dt)
{
    if (!m_state || instanceId < 0 ||
        instanceId >= static_cast<int>(m_instances.size()))
        return false;
    const Instance& instance = m_instances[instanceId];
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, instance.moduleRef);
    lua_getfield(m_state, -1, "onUpdate");
    if (!lua_isfunction(m_state, -1))
    {
        lua_pop(m_state, 2); // no onUpdate is fine, not an error
        return true;
    }
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, instance.stateRef);
    lua_pushnumber(m_state, dt);
    if (lua_pcall(m_state, 2, 0, 0) != LUA_OK)
    {
        Log::error("Lua [onUpdate " + instance.entityName +
                   "]: " + lua_tostring(m_state, -1));
        lua_pop(m_state, 2); // error object + module table
        return false;
    }
    lua_pop(m_state, 1); // module table
    return true;
}
