#include "pch.h"
#include "BufferPool.h"

bochan::BufferPool::BufferPool(size_t maxSize) : maxSize(maxSize) {}

bochan::Buffer* bochan::BufferPool::getBuffer(size_t size) {
    std::lock_guard lock(mutex);
    for (std::vector<bochan::Buffer*>::iterator it = freeBuffers.begin(); it != freeBuffers.end(); ++it) {
        if ((*it)->getSize() < size) {
            continue;
        }
        if ((*it)->getSize() == size) {
            Buffer* buff = *it;
            freeBuffers.erase(it);
            addUsed(buff);
            return buff;
        } else {
            break;
        }
    }
    Buffer* buff = addBuffer(size, false);
    if (!buff) {
        return nullptr;
    }
    addUsed(buff);
    return buff;
}

bool bochan::BufferPool::freeBuffer(Buffer* buffer) {
    return freeBuffer(buffer, false);
}

bool bochan::BufferPool::freeAndRemoveBuffer(Buffer* buffer) {
    return freeBuffer(buffer, true);
}

BOCHANAPI void bochan::BufferPool::flushUnused() {
    std::lock_guard lock(mutex);
    for (std::vector<bochan::Buffer*>::iterator it = freeBuffers.begin(); it != freeBuffers.end(); ++it) {
        deleteBuffer(*it, false);
    }
    freeBuffers.clear();
}

size_t bochan::BufferPool::getTotalSize() const {
    std::lock_guard lock(mutex);
    return byteSize;
}

size_t bochan::BufferPool::getUsedSize() const {
    std::lock_guard lock(mutex);
    return usedSize;
}

size_t bochan::BufferPool::getFreeSize() const {
    std::lock_guard lock(mutex);
    return byteSize - usedSize;
}

size_t bochan::BufferPool::getUnallocatedSize() const {
    std::lock_guard lock(mutex);
    return maxSize - byteSize;
}

size_t bochan::BufferPool::getMaxSize() const {
    return maxSize;
}

bool bochan::BufferPool::freeBuffer(Buffer* buffer, bool remove) {
    std::lock_guard lock(mutex);
    std::vector<bochan::Buffer*>::iterator it = std::find(usedBuffers.begin(), usedBuffers.end(), buffer);
    if (it == usedBuffers.end()) {
        return false;
    }
    usedBuffers.erase(it);
    usedSize -= buffer->getSize();
    if (remove) {
        deleteBuffer(buffer, false);
    } else {
        addToFreeBuffers(buffer);
    }
    return true;
}

void bochan::BufferPool::addUsed(Buffer* buffer) {
    std::lock_guard lock(mutex);
    usedBuffers.push_back(buffer);
    usedSize += buffer->getSize();
}

bochan::Buffer* bochan::BufferPool::addBuffer(size_t size, bool setFree) {
    std::lock_guard lock(mutex);
    if (byteSize + size <= maxSize) {
        Buffer* buff = new Buffer(size);
        byteSize += size;
        if (setFree) {
            addToFreeBuffers(buff);
        }
        return buff;
    }
    return nullptr;
}

void bochan::BufferPool::addToFreeBuffers(Buffer* buffer) {
    std::lock_guard lock(mutex);
    if (freeBuffers.empty() || freeBuffers[0]->getSize() >= buffer->getSize()) {
        freeBuffers.push_back(buffer);
    } else {
        for (std::vector<bochan::Buffer*>::iterator it = freeBuffers.begin(); it != freeBuffers.end(); ++it) {
            if ((*it)->getSize() <= buffer->getSize()) {
                freeBuffers.insert(it + 1, buffer);
                break;
            }
        }
    }
}

bool bochan::BufferPool::deleteBuffer(Buffer* buffer, bool checkIfFree) {
    std::lock_guard lock(mutex);
    if (checkIfFree) {
        std::vector<bochan::Buffer*>::iterator it = std::find(freeBuffers.begin(), freeBuffers.end(), buffer);
        if (it == freeBuffers.end()) {
            return false;
        }
        freeBuffers.erase(it);
    }
    byteSize -= buffer->getSize();
    delete buffer;
    return true;
}
