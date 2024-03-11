#pragma once

struct lua_State;

const char* love_getVersion();

const char* love_getCodename();

int love_initialize(lua_State* L);

int love_isVersionCompatible(lua_State* L);

int love_openNoGame(lua_State* L);

int love_openArg(lua_State* L);

int love_openCallbacks(lua_State* L);

int love_openBoot(lua_State* L);

int love_openLogfile(lua_State* L);
