#include "pch.h"
#include "Buffer.h"

bochan::Buffer::Buffer(size_t size) : data(new uint8_t[size]), size(size) {}

bochan::Buffer::~Buffer() {
    delete[] data;
    data = nullptr;
    size = 0UL;
}

uint8_t* bochan::Buffer::getPointer() const {
    return data;
}

size_t bochan::Buffer::getSize() const {
    return size;
}
