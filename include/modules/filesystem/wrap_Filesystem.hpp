#pragma once

#include "common/luax.hpp"

namespace Wrap_Filesystem
{
    int init(lua_State* L);

    int setFused(lua_State* L);

    int isFused(lua_State* L);

    int setIdentity(lua_State* L);

    int getIdentity(lua_State* L);

    int setSource(lua_State* L);

    int getSource(lua_State* L);

    int getFullCommonPath(lua_State* L);

    int getWorkingDirectory(lua_State* L);

    int getUserDirectory(lua_State* L);

    int getAppdataDirectory(lua_State* L);

    int getSaveDirectory(lua_State* L);

    int getSourceBaseDirectory(lua_State* L);

    int getRealDirectory(lua_State* L);

    int getExecutablePath(lua_State* L);

    int exists(lua_State* L);

    int getInfo(lua_State* L);

    int createDirectory(lua_State* L);

    int read(lua_State* L);

    int write(lua_State* L);

    int append(lua_State* L);

    int remove(lua_State* L);

    int setSymlinksEnabled(lua_State* L);

    int areSymlinksEnabled(lua_State* L);

    int getDirectoryItems(lua_State* L);

    int load(lua_State* L);

    int getRequirePath(lua_State* L);

    int setRequirePath(lua_State* L);

    int getInfo(lua_State* L);

    int open(lua_State* L);
} // namespace Wrap_Filesystem
