#include "modules/data/wrap_DataModule.hpp"

#include "modules/data/DataModule.hpp"

#include "modules/data/wrap_ByteData.hpp"
#include "modules/data/wrap_CompressedData.hpp"
#include "modules/data/wrap_Data.hpp"
#include "modules/data/wrap_DataView.hpp"

#include "common/b64.hpp"
#include "modules/data/ByteData.hpp"
#include "modules/data/CompressedData.hpp"
#include "modules/data/DataView.hpp"

using namespace love;

#define instance() DataModule::getInstance<DataModule>(Module::M_DATA)

int Wrap_DataModule::compress(lua_State* L)
{
    auto containerType = luax_checkcontainertype(L, 1);

    const char* formatName = luaL_checkstring(L, 2);
    auto format            = Compressor::FORMAT_LZ4;

    if (!Compressor::getConstant(formatName, format))
        luax_enumerror(L, "compressed data format", Compressor::formats, formatName);

    int level         = luaL_optinteger(L, 4, -1);
    size_t rawSize    = 0;
    const char* bytes = nullptr;

    if (lua_isstring(L, 3))
        bytes = luaL_checklstring(L, 3, &rawSize);
    else
    {
        auto* rawData = luax_checktype<Data>(L, 3);
        rawSize       = rawData->getSize();
        bytes         = (const char*)rawData->getData();
    }

    CompressedData* data = nullptr;
    luax_catchexcept(L, [&] { data = data::compress(format, bytes, rawSize, level); });

    if (containerType == data::CONTAINER_DATA)
        luax_pushtype(L, data);
    else
        lua_pushlstring(L, (const char*)data->getData(), data->getSize());

    data->release();
    return 1;
}

int Wrap_DataModule::decompress(lua_State* L)
{
    auto containerType = luax_checkcontainertype(L, 1);

    char* bytes = nullptr;
    size_t size = 0;

    if (luax_istype(L, 2, CompressedData::type))
    {
        auto* data = luax_checkcompresseddata(L, 2);
        size       = data->getDecompressedSize();
        luax_catchexcept(L, [&] { bytes = data::decompress(data, size); });
    }
    else
    {
        auto format              = Compressor::FORMAT_LZ4;
        const char* formatString = luaL_checkstring(L, 2);

        if (!Compressor::getConstant(formatString, format))
            luax_enumerror(L, "compressed data format", Compressor::formats, formatString);

        size_t compressedSize = 0;
        const char* bytes     = nullptr;

        if (luax_istype(L, 3, Data::type))
        {
            auto* data     = luax_checktype<Data>(L, 3);
            bytes          = (const char*)data->getData();
            compressedSize = data->getSize();
        }
        else
            bytes = luaL_checklstring(L, 3, &compressedSize);

        luax_catchexcept(L, [&] { bytes = data::decompress(format, bytes, compressedSize, size); });
    }

    if (containerType == data::CONTAINER_DATA)
    {
        ByteData* data = nullptr;
        luax_catchexcept(L, [&] { data = instance()->newByteData(bytes, size); });

        luax_pushtype(L, data);
        data->release();
    }
    else
    {
        lua_pushlstring(L, bytes, size);
        delete[] bytes;
    }

    return 1;
}

int Wrap_DataModule::encode(lua_State* L)
{
    auto containerType     = luax_checkcontainertype(L, 1);
    const char* formatName = luaL_checkstring(L, 2);

    data::EncodeFormat format = data::ENCODE_MAX_ENUM;
    if (!data::getConstant(formatName, format))
        luax_enumerror(L, "encode format", data::encodeFormats, formatName);

    size_t srcLength   = 0;
    const char* source = nullptr;

    if (luax_istype(L, 3, Data::type))
    {
        auto* data = luax_totype<Data>(L, 3);
        source     = (const char*)data->getData();
        srcLength  = data->getSize();
    }
    else
        source = luaL_checklstring(L, 3, &srcLength);

    size_t lineLength = luaL_optinteger(L, 4, 0);

    size_t length = 0;
    std::string dst {};

    luax_catchexcept(L, [&] { dst = data::encode(format, source, srcLength, lineLength, length); });

    if (containerType == data::CONTAINER_DATA)
    {
        ByteData* data = nullptr;

        if (!dst.empty())
            luax_catchexcept(
                L, [&] { data = instance()->newByteData((void*)dst.c_str(), length, true); });
        else
            data = instance()->newByteData(0);

        luax_pushtype(L, Data::type, data);
        data->release();
    }
    else
    {
        if (!dst.empty())
            lua_pushlstring(L, dst.c_str(), length);
        else
            lua_pushstring(L, "");
    }

    return 1;
}

int Wrap_DataModule::decode(lua_State* L)
{
    auto containerType     = luax_checkcontainertype(L, 1);
    const char* formatName = luaL_checkstring(L, 2);

    data::EncodeFormat format = data::ENCODE_MAX_ENUM;
    if (!data::getConstant(formatName, format))
        luax_enumerror(L, "decode format", data::encodeFormats, formatName);

    size_t srcLength   = 0;
    const char* source = nullptr;

    if (luax_istype(L, 3, Data::type))
    {
        auto* data = luax_totype<Data>(L, 3);
        source     = (const char*)data->getData();
        srcLength  = data->getSize();
    }
    else
        source = luaL_checklstring(L, 3, &srcLength);

    size_t dstLength                       = 0;
    std::unique_ptr<uint8_t[]> destination = nullptr;

    luax_catchexcept(L, [&] { destination = data::decode(format, source, srcLength, dstLength); });

    if (containerType == data::CONTAINER_DATA)
    {
        ByteData* data = nullptr;

        if (destination)
            luax_catchexcept(
                L, [&] { data = instance()->newByteData(destination.get(), dstLength, true); });
        else
            data = instance()->newByteData(0);

        luax_pushtype(L, Data::type, data);
        data->release();
    }
    else
    {
        if (destination)
            lua_pushlstring(L, (const char*)destination.get(), dstLength);
        else
            lua_pushstring(L, "");
    }

    return 1;
}

int Wrap_DataModule::hash(lua_State* L)
{
    auto containerType = luax_checkcontainertype(L, 1);

    auto function            = HashFunction::FUNCTION_MAX_ENUM;
    const char* formatString = luaL_checkstring(L, 2);

    if (!HashFunction::getConstant(formatString, function))
        luax_enumerror(L, "hash function", HashFunction::hashFunctions, formatString);

    HashFunction::Value value {};
    if (lua_isstring(L, 3))
    {
        size_t rawSize    = 0;
        const char* bytes = luaL_checklstring(L, 3, &rawSize);
        luax_catchexcept(L, [&] { data::hash(function, bytes, rawSize, value); });
    }
    else
    {
        auto* data = luax_checktype<Data>(L, 3);
        luax_catchexcept(L, [&] { data::hash(function, data, value); });
    }

    if (containerType == data::CONTAINER_DATA)
    {
        Data* data = nullptr;
        luax_catchexcept(L, [&] { data = instance()->newByteData(value.size); });
        std::memcpy(data->getData(), value.data, value.size);

        luax_pushtype(L, Data::type, data);
        data->release();
    }
    else
        lua_pushlstring(L, (const char*)value.data, value.size);

    return 1;
}

int Wrap_DataModule::pack(lua_State* L)
{
    if (luax_istype(L, 1, ByteData::type))
    {
        auto* byteData     = luax_checkbytedata(L, 1);
        size_t offset      = luaL_checknumber(L, 2);
        const char* format = luaL_checkstring(L, 3);

        luaL_Buffer_53 buffer {};
        lua53_str_pack(L, format, 4, &buffer);

        if (offset + buffer.nelems > byteData->getSize())
        {
            lua53_cleanupbuffer(&buffer);
            return luaL_error(L, E_DATA_PACK_OFFSET_FORMAT_PARAMS);
        }

        std::memcpy((uint8_t*)byteData->getData() + offset, (uint8_t*)buffer.ptr, buffer.nelems);

        lua53_cleanupbuffer(&buffer);
        luax_pushtype(L, Data::type, byteData);

        return 1;
    }

    auto containerType = luax_checkcontainertype(L, 1);
    const char* format = luaL_checkstring(L, 2);

    luaL_Buffer_53 buffer {};
    lua53_str_pack(L, format, 3, &buffer);

    if (containerType == data::CONTAINER_DATA)
    {
        Data* data = nullptr;
        luax_catchexcept(L, [&] { data = instance()->newByteData(buffer.nelems); });
        std::memcpy(data->getData(), buffer.ptr, buffer.nelems);

        lua53_cleanupbuffer(&buffer);
        luax_pushtype(L, Data::type, data);
        data->release();
    }
    else
        lua53_pushresult(&buffer);

    return 1;
}

int Wrap_DataModule::unpack(lua_State* L)
{
    const char* format = luaL_checkstring(L, 1);

    const char* data = nullptr;
    size_t size      = 0;

    if (luax_istype(L, 2, Data::type))
    {
        auto* _data = luax_checkdata(L, 2);
        data        = (const char*)_data->getData();
        size        = _data->getSize();
    }
    else
        data = luaL_checklstring(L, 2, &size);

    return lua53_str_unpack(L, format, data, size, 2, 3);
}

int Wrap_DataModule::newByteData(lua_State* L)
{
    ByteData* result = nullptr;

    if (luax_istype(L, 1, Data::type))
    {
        auto* data = luax_checkdata(L, 1);

        if (data->getSize() > std::numeric_limits<lua_Integer>::max())
            return luaL_error(L, "Data's size is too large!");

        lua_Integer offset = luaL_optinteger(L, 2, 0);
        if (offset < 0)
            return luaL_error(L, "Offset argument must not be negative.");

        lua_Integer size = luaL_optinteger(L, 3, data->getSize() - offset);
        if (size < 0)
            return luaL_error(L, "Size argument must be greater than zero.");
        else if ((size_t)(offset + size) > data->getSize())
            return luaL_error(L, E_OFFSET_AND_SIZE_ARGS_FIT_WITHIN_DATA);

        const char* bytes = (const char*)data->getData() + offset;
        luax_catchexcept(L, [&] { result = instance()->newByteData(bytes, (size_t)size); });
    }
    else if (lua_type(L, 1) == LUA_TSTRING)
    {
        size_t size      = 0;
        const char* data = luaL_checklstring(L, 1, &size);
        luax_catchexcept(L, [&] { result = instance()->newByteData(data, size); });
    }
    else
    {
        lua_Integer size = luaL_checkinteger(L, 1);
        if (size <= 0)
            return luaL_error(L, E_DATA_SIZE_MUST_BE_POSITIVE);

        luax_catchexcept(L, [&] { result = instance()->newByteData((size_t)size); });
    }

    luax_pushtype(L, result);
    result->release();

    return 1;
}

int Wrap_DataModule::newDataView(lua_State* L)
{
    auto* data = luax_checkdata(L, 1);

    lua_Integer offset = luaL_checkinteger(L, 2);
    lua_Integer size   = luaL_checkinteger(L, 3);

    if (offset < 0 || size < 0)
        return luaL_error(L, "DataView offset and size must not be negative.");

    DataView* result = nullptr;
    luax_catchexcept(L, [&] { result = instance()->newDataView(data, offset, size); });

    luax_pushtype(L, result);
    result->release();

    return 1;
}

// clang-format off
static constexpr luaL_Reg functions[] =
{
    { "compress",      Wrap_DataModule::compress    },
    { "decompress",    Wrap_DataModule::decompress  },
    { "encode",        Wrap_DataModule::encode      },
    { "decode",        Wrap_DataModule::decode      },
    { "hash",          Wrap_DataModule::hash        },
    { "pack",          Wrap_DataModule::pack        },
    { "unpack",        Wrap_DataModule::unpack      },
    { "getPackedSize", lua53_str_packsize           },
    { "newByteData",   Wrap_DataModule::newByteData },
    { "newDataView",   Wrap_DataModule::newDataView }
};

static constexpr lua_CFunction types[] =
{
    love::open_data,
    love::open_bytedata,
    love::open_dataview,
    love::open_compresseddata,
    nullptr
};
// clang-format on

int Wrap_DataModule::open(lua_State* L)
{
    auto* instance = instance();

    if (instance == nullptr)
        luax_catchexcept(L, [&] { instance = new DataModule(); });
    else
        instance->retain();

    WrappedModule module {};
    module.instance  = instance;
    module.name      = "data";
    module.type      = &Module::type;
    module.functions = functions;
    module.types     = types;

    return luax_register_module(L, module);
}

namespace love
{
    data::ContainerType luax_checkcontainertype(lua_State* L, int index)
    {
        const char* containerString = luaL_checkstring(L, index);
        auto containerType          = data::CONTAINER_MAX_ENUM;

        if (!data::getConstant(containerString, containerType))
            luax_enumerror(L, "data container type", data::containerTypes, containerString);

        return containerType;
    }
} // namespace love
