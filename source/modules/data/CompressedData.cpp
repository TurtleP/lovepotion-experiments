#include "modules/data/CompressedData.hpp"

#include "common/Exception.hpp"

namespace love
{
    Type CompressedData::type("CompressedData", &Data::type);

    CompressedData::CompressedData(Compressor::Format format, char* data, size_t size,
                                   size_t rawSize, bool own) :
        format(format),
        data(nullptr),
        dataSize(size),
        originalSize(rawSize)
    {
        if (own)
        {
            this->data.reset(data);
        }
        else
        {
            try
            {
                this->data = std::make_unique<char[]>(size);
            }
            catch (std::bad_alloc&)
            {
                throw love::Exception(E_OUT_OF_MEMORY);
            }

            std::copy_n(data, size, this->data.get());
        }
    }

    CompressedData::CompressedData(const CompressedData& other) :
        format(other.format),
        rawSize(other.rawSize),
        data(nullptr),
        dataSize(other.dataSize),
        originalSize(other.originalSize)
    {
        try
        {
            this->data = std::make_unique<char[]>(this->dataSize);
        }
        catch (std::bad_alloc&)
        {
            throw love::Exception(E_OUT_OF_MEMORY);
        }

        std::copy_n(other.data.get(), this->dataSize, this->data.get());
    }

    CompressedData* CompressedData::clone() const
    {
        return new CompressedData(*this);
    }

    Compressor::Format CompressedData::getFormat() const
    {
        return this->format;
    }

    size_t CompressedData::getDecompressedSize() const
    {
        return this->originalSize;
    }

    void* CompressedData::getData() const
    {
        return this->data.get();
    }

    size_t CompressedData::getSize() const
    {
        return this->dataSize;
    }
} // namespace love
