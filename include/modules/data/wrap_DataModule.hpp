#pragma once

#include "common/luax.hpp"
#include "modules/data/DataModule.hpp"

namespace love
{
    DataModule::ContainerType luax_checkcontainertype(lua_State* L, int index);

    int open_datamodule(lua_State* L);
} // namespace love

namespace Wrap_DataModule
{
    int compress(lua_State* L);

    int decompress(lua_State* L);
} // namespace Wrap_DataModule
