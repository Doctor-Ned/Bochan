#pragma once
#include "pch.h"

namespace bochan {
    template <typename T>
    class Buffer sealed {
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

    template <typename T> bochan::Buffer<T>::Buffer(size_t size) : data(new T[size]), size(size), byteSize(size * sizeof(T)) {}

    template <typename T> bochan::Buffer<T>::~Buffer() {
        delete[] data;
        data = nullptr;
        size = 0UL;
    }

    template <typename T> T* bochan::Buffer<T>::getPointer() const {
        return data;
    }

    template <typename T> size_t bochan::Buffer<T>::getSize() const {
        return size;
    }

    template<typename T>
    size_t bochan::Buffer<T>::getByteSize() const {
        return byteSize;
    }

    using ByteBuffer = Buffer<uint8_t>;
}
