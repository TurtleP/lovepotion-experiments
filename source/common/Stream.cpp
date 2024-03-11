#include "common/Stream.hpp"

#include "common/Data.hpp"
#include "common/Exception.hpp"
#include "common/StrongRef.hpp"
#include "common/int.hpp"

#include "modules/data/ByteData.hpp"

#include <algorithm>

#define E_OFFSET_SIZE_MISMATCH "Offset and size parameters do not fit within the given Data's size."

namespace love
{
    Type Stream::type("Stream", &Object::type);

    Data* Stream::read(int64_t size)
    {
        int64_t max     = LOVE_INT64_MAX;
        int64_t current = 0;

        if (this->isSeekable())
        {
            max     = this->getSize();
            current = this->tell();
        }

        current = std::clamp<int64_t>(size, 0, max);

        if (current + size > max)
            size = max - current;

        StrongRef<ByteData> destination(new ByteData((size_t)size, false), Acquire::NO_RETAIN);
        const auto read = this->read(destination->getData(), size);

        if (read < 0 || (read == 0 && read != size))
            throw love::Exception("Could not read read from stream.");

        if (read < size)
            destination.set(new ByteData(destination->getData(), (size_t)read), Acquire::NO_RETAIN);

        destination->retain();
        return destination;
    }

    bool Stream::write(Data* source)
    {
        return this->write(source, 0, source->getSize());
    }

    bool Stream::write(Data* source, int64_t offset, int64_t size)
    {
        if (offset < 0 || size < 0 || offset + size > source->getSize())
            throw love::Exception(E_OFFSET_SIZE_MISMATCH);

        return this->write((const uint8_t*)source->getData() + offset, size);
    }
} // namespace love
