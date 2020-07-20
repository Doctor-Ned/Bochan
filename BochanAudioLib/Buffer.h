#pragma once
#include "pch.h"

namespace bochan {
    class BOCHANAPI Buffer sealed {
    public:
        Buffer(size_t size);
        ~Buffer();
        Buffer(Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(Buffer&) = delete;
        Buffer& operator=(Buffer&&) = delete;
        uint8_t* getPointer() const;
        size_t getSize() const;
    private:
        uint8_t* data;
        size_t size;
    };
}
