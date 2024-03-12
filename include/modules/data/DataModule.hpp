#pragma once

#include "common/Module.hpp"

#include "modules/data/ByteData.hpp"
#include "modules/data/DataView.hpp"

#include "utility/map.hpp"

namespace love
{
    class DataModule : public Module
    {
      public:
        enum EncodeFormat
        {
            ENCODE_BASE64,
            ENCODE_HEX,
            ENCODE_MAX_ENUM
        };

        enum ContainerType
        {
            CONTAINER_DATA,
            CONTAINER_STRING,
            CONTAINER_MAX_ENUM
        };

        DataModule();

        virtual ~DataModule();

        CompressedData* compress(const void* data, size_t size,
                                 CompressedData::Format format) const;

        char* decompress(CompressedData* data, size_t& size);

        DataView* newDataView(Data* data, size_t offset, size_t size) const;

        ByteData* newByteData(size_t size) const;

        ByteData* newByteData(const void* data, size_t size) const;

        ByteData* newByteData(void* data, size_t size, bool own) const;

        // clang-format off
        STRINGMAP_DECLARE(encodeFormats, EncodeFormat,
            { "base64", ENCODE_BASE64 },
            { "hex",    ENCODE_HEX    }
        );

        STRINGMAP_DECLARE(containerTypes, ContainerType,
            { "data",   CONTAINER_DATA   },
            { "string", CONTAINER_STRING }
        );
        // clang-format on
    };
} // namespace love
