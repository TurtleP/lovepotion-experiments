#include "modules/filesystem/FileData.hpp"

#include <algorithm>
#include <filesystem>
#include <limits>

namespace love
{
    Type FileData::type("FileData", &Data::type);

    FileData::FileData(uint64_t size, std::string_view filename) :
        data(nullptr),
        size(size),
        filename(filename)
    {
        try
        {
            this->data = new char[(size_t)size];
        }
        catch (std::bad_alloc&)
        {
            throw love::Exception(E_OUT_OF_MEMORY);
        }

        const auto path = std::filesystem::path(filename);

        if (path.has_extension())
        {
            this->extension = path.extension().string().substr(1);
            this->name      = path.filename().c_str();
        }
        else
            this->name = path.filename().c_str();
    }

    FileData::FileData(const FileData& other) :
        data(nullptr),
        size(other.size),
        filename(other.filename),
        extension(other.extension),
        name(other.name)
    {
        try
        {
            this->data = new char[(size_t)this->size];
        }
        catch (std::bad_alloc&)
        {
            throw love::Exception(E_OUT_OF_MEMORY);
        }

        std::copy_n((uint8_t*)other.data, this->size, (uint8_t*)this->data);
    }

    FileData::~FileData()
    {
        delete[] this->data;
    }

    FileData* FileData::clone() const
    {
        return new FileData(*this);
    }

    void* FileData::getData() const
    {
        return this->data;
    }

    size_t FileData::getSize() const
    {
        size_t max = std::numeric_limits<size_t>::max();
        return size > max ? max : (size_t)size;
    }

    const std::string& FileData::getFilename() const
    {
        return this->filename;
    }

    const std::string& FileData::getExtension() const
    {
        return this->extension;
    }

    const std::string& FileData::getName() const
    {
        return this->name;
    }
} // namespace love
