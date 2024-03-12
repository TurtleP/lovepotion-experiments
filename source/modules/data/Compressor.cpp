#include "modules/data/Compressor.hpp"

#include "common/Exception.hpp"
#include "common/int.hpp"

#include <lz4.h>
#include <lz4hc.h>

#include <zlib.h>

namespace love
{
    class LZ4Compressor : public Compressor
    {
        char* compress(Compressor::Format format, const char* data, size_t dataSize, int level,
                       size_t& compressedSize) override
        {
            return nullptr;
        }

        char* decompress(Compressor::Format format, const char* data, size_t dataSize,
                         size_t& decompressedSize) override
        {
            return nullptr;
        }

        bool isSupported(Compressor::Format format) const override
        {
            return format == Compressor::FORMAT_LZ4;
        }
    };

    class ZlibCompressor : public Compressor
    {
        char* compress(Compressor::Format format, const char* data, size_t dataSize, int level,
                       size_t& compressedSize) override
        {
            return nullptr;
        }

        char* decompress(Compressor::Format format, const char* data, size_t dataSize,
                         size_t& decompressedSize) override
        {
            return nullptr;
        }

        bool isSupported(Compressor::Format format) const override
        {
            return format == Compressor::FORMAT_GZIP || format == Compressor::FORMAT_ZLIB ||
                   format == Compressor::FORMAT_DEFLATE;
        }
    };

    Compressor* Compressor::getCompressor(Compressor::Format format)
    {
        static LZ4Compressor lz4;
        static ZlibCompressor zlib;

        Compressor* compressors[] = { &lz4, &zlib };
        for (auto* compressor : compressors)
        {
            if (compressor->isSupported(format))
                return compressor;
        }
    }
} // namespace love
