#include "pch.h"
#include "Buffer.h"

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
