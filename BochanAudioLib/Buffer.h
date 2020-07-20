#pragma once
#include "pch.h"

namespace bochan {
    template <typename T>
    class BOCHANAPI Buffer sealed {
    public:
        Buffer(size_t size);
        ~Buffer();
        Buffer(Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(Buffer&) = delete;
        Buffer& operator=(Buffer&&) = delete;
        T* getPointer() const;
        size_t getSize() const;
        size_t getByteSize() const;
    private:
        T* data;
        size_t size, byteSize;
    };

    using ByteBuffer = Buffer<uint8_t>;
}
