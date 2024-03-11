#include "common/luax.hpp"
#include "common/version.hpp"

#include "boot.hpp"
#include "modules/love/love.hpp"

#include <vector>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

enum DoneAction
{
    DONE_QUIT,
    DONE_RESTART
};

static DoneAction runLove(std::vector<const char*>& args, int& result, love::Variant& restartValue)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_bit(L);

    love::luax_preload(L, love_initialize, "love");

    {
        lua_newtable(L);

        if (args.size() > 0)
        {
            lua_pushstring(L, args[0]);
            lua_rawseti(L, -2, -2);
        }

        lua_pushstring(L, "embedded boot.lua");
        lua_rawseti(L, -2, -1);

        for (int index = 1; index < args.size(); index++)
        {
            lua_pushstring(L, args[index]);
            lua_rawseti(L, -2, index);
        }

        lua_setglobal(L, "arg");
    }

    love::luax_require(L, "love");

    {
        lua_pushboolean(L, 1);
        lua_setfield(L, -2, "_exe");
    }

    love::luax_pushvariant(L, restartValue);
    lua_setfield(L, -2, "_restart");
    restartValue = love::Variant();

    lua_pop(L, 1);

    love::luax_require(L, "love.boot");

    lua_newthread(L);
    lua_pushvalue(L, -2);

    int position = lua_gettop(L);
    int results  = 0;

    while (love::mainLoop(L, 0, &results))
        lua_pop(L, lua_gettop(L) - position);

    result            = 0;
    DoneAction action = DONE_QUIT;

    int index = position;
    if (!lua_isnoneornil(L, index))
    {
        if (lua_type(L, index) == LUA_TSTRING &&
            std::strncmp(lua_tostring(L, index), "restart", 7) == 0)
        {
            action = DONE_RESTART;
        }
        else if (lua_isnumber(L, index))
            result = lua_tonumber(L, index);

        if (index < lua_gettop(L))
            restartValue = love::luax_checkvariant(L, index + 1, false);
    }

    lua_close(L);

    return action;
}

int main(int argc, char** argv)
{
    int result        = 0;
    DoneAction action = DONE_QUIT;
    love::Variant restartValue;

    // "'lovepotion_path' 'embedded boot.lua' 'game'"
    std::vector<const char*> args(argv, argv + argc);
    args.push_back("game");

    do
    {
        action = runLove(args, result, restartValue);
    } while (action != DONE_QUIT);

    return result;
}
