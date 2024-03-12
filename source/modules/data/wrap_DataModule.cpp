#include "modules/data/wrap_DataModule.hpp"

using namespace love;

namespace love
{
    DataModule::ContainerType luax_checkcontainertype(lua_State* L, int index)
    {
        const char* containerString = luaL_checkstring(L, index);
        auto containerType          = DataModule::CONTAINER_MAX_ENUM;

        if (!DataModule::getConstant(containerString, containerType))
            luax_enumerror(L, "data container type", DataModule::containerTypes, containerString);

        return containerType;
    }

    int open_datamodule(lua_State* L)
    {}
} // namespace love
