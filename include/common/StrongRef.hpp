#pragma once

#include "common/Object.hpp"

namespace love
{
    template<typename T>
    class StrongRef
    {
      public:
        StrongRef() : object(nullptr)
        {}

        StrongRef(T* object, Acquire acquire = Acquire::RETAIN) : object(object)
        {
            if (object && acquire == Acquire::RETAIN)
                object->retain();
        }

        StrongRef(const StrongRef& other) : object(other.get())
        {
            if (object)
                object->retain();
        }

        StrongRef(StrongRef&& other) : object(other.object)
        {
            other.object = nullptr;
        }

        ~StrongRef()
        {
            if (object)
                object->release();
        }

        StrongRef& operator=(const StrongRef& other)
        {
            this->set(other.get());
            return *this;
        }

        T* operator->() const
        {
            return this->object;
        }

        explicit operator bool() const
        {
            return this->object != nullptr;
        }

        operator T*() const
        {
            return this->object;
        }

        void set(T* _object, Acquire acquire = Acquire::RETAIN)
        {
            if (_object && acquire == Acquire::RETAIN)
                _object->retain();

            if (this->object)
                this->object->release();

            this->object = object;
        }

        T* get() const
        {
            return this->object;
        }

      private:
        T* object;
    };
} // namespace love
