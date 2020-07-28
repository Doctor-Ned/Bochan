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
        size_t getTotalSize() const;
        size_t getTotalByteSize() const;
        size_t getUsedSize() const;
        size_t getUsedByteSize() const;
        void setUsedSize(size_t usedSize);
        void setUsedByteSize(size_t usedByteSize);
    private:
        T* data;
        size_t size, byteSize, usedSize;
    };

    template <typename T> bochan::Buffer<T>::Buffer(size_t size) : data(new T[size]), size(size), byteSize(size * sizeof(T)), usedSize(byteSize) {}

    template <typename T> bochan::Buffer<T>::~Buffer() {
        delete[] data;
        data = nullptr;
        size = 0UL;
    }

    template <typename T> T* bochan::Buffer<T>::getPointer() const {
        return data;
    }

    template <typename T> size_t bochan::Buffer<T>::getTotalSize() const {
        return size;
    }

    template<typename T>
    size_t bochan::Buffer<T>::getTotalByteSize() const {
        return byteSize;
    }

    template<typename T>
    size_t bochan::Buffer<T>::getUsedSize() const {
        return usedSize;
    }

    template<typename T>
    size_t bochan::Buffer<T>::getUsedByteSize() const {
        return usedSize * sizeof(T);
    }

    template<typename T>
    void bochan::Buffer<T>::setUsedSize(size_t usedSize) {
        assert(usedSize >= 0ULL && usedSize <= size);
        this->usedSize = usedSize;
    }

    template<typename T>
    void bochan::Buffer<T>::setUsedByteSize(size_t usedByteSize) {
        assert(usedSize >= 0ULL && usedSize <= byteSize);
        this->usedSize = usedByteSize / sizeof(T);
    }

    using ByteBuffer = Buffer<uint8_t>;
}
