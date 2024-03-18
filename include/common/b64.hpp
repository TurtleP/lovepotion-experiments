#pragma once

#include <memory>
#include <string>

#include <stddef.h>

namespace love
{
    std::string b64_encode(const char* source, size_t sourceLength, size_t lineLength,
                           size_t& destinationLength);

    std::unique_ptr<uint8_t[]> b64_decode(const char* source, size_t sourceLength, size_t& size);
} // namespace love
