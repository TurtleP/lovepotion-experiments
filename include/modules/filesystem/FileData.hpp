#pragma once

#include "common/Data.hpp"
#include "common/Exception.hpp"
#include "common/int.hpp"

#include <memory>
#include <string>

namespace love
{
    class FileData : public Data
    {
      public:
        static Type type;

        FileData(uint64_t size, std::string_view filename);

        FileData(const FileData& other);

        virtual ~FileData()
        {}

        FileData* clone() const override;

        void* getData() const override;

        size_t getSize() const override;

        std::string_view getFilename() const;

        std::string_view getExtension() const;

        std::string_view getName() const;

      private:
        std::unique_ptr<char[]> data;
        uint64_t size;

        std::string_view filename;
        std::string_view extension;
        std::string_view name;
    };
} // namespace love
