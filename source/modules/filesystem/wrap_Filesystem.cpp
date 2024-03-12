#include "modules/filesystem/wrap_Filesystem.hpp"

#include "modules/data/wrap_Data.hpp"
#include "modules/filesystem/physfs/Filesystem.hpp"

#include "modules/filesystem/wrap_File.hpp"
#include "modules/filesystem/wrap_FileData.hpp"

using namespace love;

#define instance() (Module::getInstance<Filesystem>(Module::M_FILESYSTEM))

#include <algorithm>
#include <string_view>

static std::size_t replaceAll(std::string& inout, std::string_view what, std::string_view with)
{
    std::size_t count          = 0;
    std::string::size_type pos = 0;

    while ((pos = inout.find(what, pos)) != std::string::npos)
    {
        inout.replace(pos, what.length(), with);
        pos += with.length();
        ++count;
    }

    return count;
}

int Wrap_Filesystem::init(lua_State* L)
{
    const char* arg0 = luaL_checkstring(L, 1);
    luax_catchexcept(L, [&] { instance()->init(arg0); });

    return 0;
}

int Wrap_Filesystem::setFused(lua_State* L)
{
    bool fused = luax_toboolean(L, 1);
    instance()->setFused(fused);

    return 0;
}

int Wrap_Filesystem::isFused(lua_State* L)
{
    luax_pushboolean(L, instance()->isFused());

    return 1;
}

int Wrap_Filesystem::setIdentity(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    bool append      = luax_optboolean(L, 2, false);

    if (!instance()->setIdentity(name, append))
        return luaL_error(L, "Could not set write directory to '%s'.", name);

    return 0;
}

int Wrap_Filesystem::getIdentity(lua_State* L)
{
    luax_pushstring(L, instance()->getIdentity());

    return 1;
}

int Wrap_Filesystem::setSource(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);

    if (!instance()->setSource(name))
        return luaL_error(L, "Could not set source directory to '%s'.", name);

    return 0;
}

int Wrap_Filesystem::getSource(lua_State* L)
{
    luax_pushstring(L, instance()->getSource());

    return 1;
}

int Wrap_Filesystem::mount(lua_State* L)
{
    std::string archive {};
    if (luax_istype(L, 1, Data::type))
    {
        Data* data = luax_checkdata(L, 1);
        int start  = 2;

        if (luax_istype(L, 1, FileData::type))
        {
            FileData* fileData = luax_checkfiledata(L, 1);
            archive            = fileData->getFilename();
            start              = 2;
        }
    }
}

int Wrap_Filesystem::mountFullPath(lua_State* L)
{}

int Wrap_Filesystem::mountCommonPath(lua_State* L)
{}

int Wrap_Filesystem::unmount(lua_State* L)
{}

int Wrap_Filesystem::unmountFullPath(lua_State* L)
{}

int Wrap_Filesystem::unmountCommonPath(lua_State* L)
{}

int Wrap_Filesystem::openFile(lua_State* L)
{
    const char* filename   = luaL_checkstring(L, 1);
    const char* modeString = luaL_checkstring(L, 2);

    File::Mode mode = File::MODE_CLOSED;
    if (!File::getConstant(modeString, mode))
        return luaL_error(L, "Invalid file mode '%s'.", modeString);

    File* file = nullptr;
    try
    {
        file = instance()->openFile(filename, mode);
    }
    catch (love::Exception& e)
    {
        return luax_ioerror(L, "%s", e.what());
    }

    luax_pushtype(L, file);
    file->release();

    return 1;
}

int Wrap_Filesystem::getFullCommonPath(lua_State* L)
{}

int Wrap_Filesystem::getWorkingDirectory(lua_State* L)
{
    luax_pushstring(L, instance()->getWorkingDirectory());

    return 1;
}

int Wrap_Filesystem::getUserDirectory(lua_State* L)
{
    luax_pushstring(L, instance()->getUserDirectory());

    return 1;
}

int Wrap_Filesystem::getAppdataDirectory(lua_State* L)
{
    luax_pushstring(L, instance()->getAppdataDirectory());

    return 1;
}

int Wrap_Filesystem::getSaveDirectory(lua_State* L)
{
    luax_pushstring(L, instance()->getSaveDirectory());

    return 1;
}

int Wrap_Filesystem::getSourceBaseDirectory(lua_State* L)
{
    luax_pushstring(L, instance()->getSourceBaseDirectory());

    return 1;
}

int Wrap_Filesystem::getRealDirectory(lua_State* L)
{
    const char* filepath = luaL_checkstring(L, 1);
    std::string directory {};

    try
    {
        directory = instance()->getRealDirectory(filepath);
    }
    catch (love::Exception& e)
    {
        return luax_ioerror(L, "%s", e.what());
    }

    luax_pushstring(L, directory);
    return 1;
}

int Wrap_Filesystem::getExecutablePath(lua_State* L)
{
    luax_pushstring(L, instance()->getExecutablePath());

    return 1;
}

int Wrap_Filesystem::createDirectory(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    luax_pushboolean(L, instance()->createDirectory(path));

    return 1;
}

int Wrap_Filesystem::remove(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    luax_pushboolean(L, instance()->remove(path));

    return 1;
}

int Wrap_Filesystem::read(lua_State* L)
{}

static int write_or_append(lua_State* L, File::Mode mode)
{
    const char* filename = luaL_checkstring(L, 1);

    const char* input = nullptr;
    size_t length     = 0;

    if (luax_istype(L, 2, Data::type))
    {
        Data* data = luax_totype<Data>(L, 2);
        input      = (const char*)data->getData();
        length     = data->getSize();
    }
    else if (lua_isstring(L, 2))
        input = lua_tolstring(L, 2, &length);
    else
        return luaL_argerror(L, 2, "string or Data expected");

    length = luaL_optinteger(L, 3, length);

    try
    {
        if (mode == File::MODE_APPEND)
            instance()->append(filename, input, length);
        else
            instance()->write(filename, input, length);
    }
    catch (love::Exception& e)
    {
        return luax_ioerror(L, "%s", e.what());
    }

    luax_pushboolean(L, true);
    return 1;
}

int Wrap_Filesystem::write(lua_State* L)
{
    return write_or_append(L, File::MODE_WRITE);
}

int Wrap_Filesystem::append(lua_State* L)
{
    return write_or_append(L, File::MODE_APPEND);
}

int Wrap_Filesystem::getDirectoryItems(lua_State* L)
{}

int Wrap_Filesystem::lines(lua_State* L)
{}

int Wrap_Filesystem::exists(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    lua_pushboolean(L, instance()->exists(filename));

    return 1;
}

int Wrap_Filesystem::load(lua_State* L)
{
    std::string filename      = luaL_checkstring(L, 1);
    Filesystem::LoadMode mode = Filesystem::LOADMODE_ANY;

    if (!lua_isnoneornil(L, 2))
    {
        const char* modestr = luaL_checkstring(L, 2);
        if (!Filesystem::getConstant(modestr, mode))
            return luaL_error(L, "Invalid load mode '%s'.", modestr);
    }

    // TODO: file data
    Data* data = nullptr;

    try
    {
        data = instance()->read(filename.c_str());
    }
    catch (love::Exception& e)
    {
        return luax_ioerror(L, "%s", e.what());
    }

    int status = 0;
#if (LUA_VERSION_NUM > 501) || defined(LUA_JITLIBNAME)
    const char* modeStr = nullptr;
    Filesystem::getConstant(mode, modeStr);

    status = luaL_loadbufferx(L, (const char*)data->getData(), data->getSize(), filename.c_str(),
                              modeStr);
#else
    if (mode == Filesystem::LOADMODE_ANY)
        status =
            luaL_loadbuffer(L, (const char*)data->getData(), data->getSize(), filename.c_str());
    else
    {
        data->release();
        return luaL_error(L, "only \"bt\" is supported on this Lua interpreter\n");
    }
#endif

    data->release();

    switch (status)
    {
        case LUA_ERRMEM:
            return luaL_error(L, "Memory allocation error: %s", lua_tostring(L, -1));
        case LUA_ERRSYNTAX:
            return luaL_error(L, "Syntax error: %s", lua_tostring(L, -1));
        default:
            return 1;
    }
}

int Wrap_Filesystem::getInfo(lua_State* L)
{
    const char* filepath = luaL_checkstring(L, 1);
    Filesystem::Info info {};

    int start                   = 2;
    Filesystem::FileType filter = Filesystem::FILETYPE_MAX_ENUM;

    if (lua_isstring(L, start))
    {
        const char* type = luaL_checkstring(L, start);
        if (!Filesystem::getConstant(type, filter))
            return luaL_error(L, "Invalid file type '%s'.", type);

        start++;
    }

    if (instance()->getInfo(filepath, info))
    {
        if (filter != Filesystem::FILETYPE_MAX_ENUM && info.type != filter)
        {
            lua_pushnil(L);
            return 1;
        }

        std::string_view type;
        if (!Filesystem::getConstant(info.type, type))
            return luaL_error(L, "Invalid file type '%d'.", info.type);

        if (lua_istable(L, start))
            lua_pushvalue(L, start);
        else
            lua_createtable(L, 0, 3);

        luax_pushstring(L, type);
        lua_setfield(L, -2, "type");

        luax_pushboolean(L, info.readonly);
        lua_setfield(L, -2, "readonly");

        info.size = std::min(info.size, 0x20000000000000LL);
        if (info.size >= 0)
        {
            lua_pushnumber(L, (lua_Number)info.size);
            lua_setfield(L, -2, "size");
        }

        info.modtime = std::min(info.modtime, 0x20000000000000LL);
        if (info.modtime >= 0)
        {
            lua_pushnumber(L, (lua_Number)info.modtime);
            lua_setfield(L, -2, "modtime");
        }
    }
    else
        lua_pushnil(L);

    return 1;
}

int Wrap_Filesystem::setSymlinksEnabled(lua_State* L)
{
    bool enable = luax_toboolean(L, 1);
    instance()->setSymlinksEnabled(enable);

    return 0;
}

int Wrap_Filesystem::areSymlinksEnabled(lua_State* L)
{
    luax_pushboolean(L, instance()->areSymlinksEnabled());

    return 1;
}

int Wrap_Filesystem::newFileData(lua_State* L)
{}

int Wrap_Filesystem::getRequirePath(lua_State* L)
{
    std::string path;
    bool separator = false;

    for (const auto& element : instance()->getRequirePath())
    {
        if (separator)
            path += ";";
        else
            separator = true;

        path += element;
    }

    luax_pushstring(L, path.c_str());
    return 1;
}

int Wrap_Filesystem::setRequirePath(lua_State* L)
{
    std::string element = luax_checkstring(L, 1);
    auto& requirePath   = instance()->getRequirePath();

    requirePath.clear();

    size_t startPos = 0;
    size_t endPos   = element.find(';');

    while (endPos != std::string::npos)
    {
        requirePath.push_back(element.substr(startPos, endPos - startPos));
        startPos = endPos + 1;
        endPos   = element.find(';', startPos);
    }

    requirePath.push_back(element.substr(startPos));

    return 0;
}

static int loader(lua_State* L)
{
    std::string moduleName = luaL_checkstring(L, 1);

    for (char& c : moduleName)
    {
        if (c == '.')
            c = '/';
    }

    auto* instance = instance();

    for (std::string element : instance->getRequirePath())
    {
        replaceAll(element, "?", moduleName);

        Filesystem::Info info {};
        if (instance->getInfo(element.c_str(), info) && info.type != Filesystem::FILETYPE_DIRECTORY)
        {
            lua_pop(L, 1);
            lua_pushstring(L, element.c_str());

            return Wrap_Filesystem::load(L);
        }
    }

    lua_pushfstring(L, E_NO_FILE_IN_LOVE_DIRS, moduleName.c_str());
    return 1;
}

// clang-format off
static constexpr luaL_Reg functions[]
{
    { "append",                 Wrap_Filesystem::append                 },
    { "exists",                 Wrap_Filesystem::exists                 },
    { "getAppdataDirectory",    Wrap_Filesystem::getAppdataDirectory    },
    { "getExecutablePath",      Wrap_Filesystem::getExecutablePath      },
    { "getIdentity",            Wrap_Filesystem::getIdentity            },
    { "getInfo",                Wrap_Filesystem::getInfo                },
    { "getRealDirectory",       Wrap_Filesystem::getRealDirectory       },
    { "getSaveDirectory",       Wrap_Filesystem::getSaveDirectory       },
    { "getSource",              Wrap_Filesystem::getSource              },
    { "getSourceBaseDirectory", Wrap_Filesystem::getSourceBaseDirectory },
    { "getUserDirectory",       Wrap_Filesystem::getUserDirectory       },
    { "getWorkingDirectory",    Wrap_Filesystem::getWorkingDirectory    },
    { "init",                   Wrap_Filesystem::init                   },
    { "isFused",                Wrap_Filesystem::isFused                },
    { "load",                   Wrap_Filesystem::load                   },
    { "setFused",               Wrap_Filesystem::setFused               },
    { "setIdentity",            Wrap_Filesystem::setIdentity            },
    { "setSource",              Wrap_Filesystem::setSource              },
    { "write",                  Wrap_Filesystem::write                  }
};

static constexpr lua_CFunction types[] =
{
    love::open_filedata,
    love::open_file,
    nullptr
};
// clang-format on

int Wrap_Filesystem::open(lua_State* L)
{
    Filesystem* instance = instance();

    if (instance == nullptr)
        luax_catchexcept(L, [&] { instance = new Filesystem(); });
    else
        instance->retain();

    luax_register_searcher(L, loader, 2);

    WrappedModule module {};
    module.instance  = instance;
    module.name      = "filesystem";
    module.type      = &Module::type;
    module.functions = functions;
    module.types     = types;

    return luax_register_module(L, module);
}
