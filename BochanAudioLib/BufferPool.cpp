#include "pch.h"
#include "BufferPool.h"

bochan::BufferPool::BufferPool(size_t maxSize) : maxSize(maxSize) {}

bochan::BufferPool::~BufferPool() {
    std::lock_guard lock(mutex);
    BOCHAN_DEBUG("Clearing buffer pool. {}UB,{}FB,{}US,{}FS,{}TS", usedBuffers.size(), freeBuffers.size(), usedSize, getFreeSize(), byteSize);;
    for (ByteBuffer* buff : usedBuffers) {
        freeBuffer(buff);
    }
    flushUnused();
}

bochan::ByteBuffer* bochan::BufferPool::getBuffer(size_t size) {
    std::lock_guard lock(mutex);
    for (std::vector<bochan::ByteBuffer*>::iterator it = freeBuffers.begin(); it != freeBuffers.end(); ++it) {
        if ((*it)->getTotalSize() < size) {
            continue;
        }
        ByteBuffer* buff = *it;
        buff->setUsedSize(size);
        freeBuffers.erase(it);
        addUsed(buff);
        BOCHAN_TRACE("Reusing buffer {:x} of size {} for desired size {}.", reinterpret_cast<uintptr_t>(buff), buff->getTotalByteSize(), size);
        return buff;
    }
    ByteBuffer* buff = addBuffer(size, false);
    if (!buff) {
        return nullptr;
    }
    addUsed(buff);
    return buff;
}

bool bochan::BufferPool::freeBuffer(ByteBuffer* buffer) {
    return freeBuffer(buffer, false);
}

bool bochan::BufferPool::freeAndRemoveBuffer(ByteBuffer* buffer) {
    return freeBuffer(buffer, true);
}

BOCHANAPI void bochan::BufferPool::flushUnused() {
    std::lock_guard lock(mutex);
    for (std::vector<bochan::ByteBuffer*>::iterator it = freeBuffers.begin(); it != freeBuffers.end(); ++it) {
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

bool bochan::BufferPool::freeBuffer(ByteBuffer* buffer, bool remove) {
    std::lock_guard lock(mutex);
    BOCHAN_TRACE("Freeing buffer {:x} of size {} ({}).", reinterpret_cast<uintptr_t>(buffer), buffer->getTotalByteSize(), remove);
    std::vector<bochan::ByteBuffer*>::iterator it = std::find(usedBuffers.begin(), usedBuffers.end(), buffer);
    if (it == usedBuffers.end()) {
        return false;
    }
    usedBuffers.erase(it);
    usedSize -= buffer->getTotalSize();
    if (remove) {
        deleteBuffer(buffer, false);
    } else {
        addToFreeBuffers(buffer);
    }
    return true;
}

void bochan::BufferPool::addUsed(ByteBuffer* buffer) {
    std::lock_guard lock(mutex);
    usedBuffers.push_back(buffer);
    usedSize += buffer->getTotalSize();
}

bochan::ByteBuffer* bochan::BufferPool::addBuffer(size_t size, bool setFree) {
    std::lock_guard lock(mutex);
    if (byteSize + size <= maxSize) {
        ByteBuffer* buff = new ByteBuffer(size);
        BOCHAN_DEBUG("Adding new buffer {:x} of size {} ({})", reinterpret_cast<uintptr_t>(buff), size, setFree);
        byteSize += size;
        if (setFree) {
            addToFreeBuffers(buff);
        }
        return buff;
    }
    BOCHAN_ERROR("Attempting to overflow the buffer pool by adding {} bytes (the total size is {}, max size is {})", size, byteSize, maxSize);
    return nullptr;
}

void bochan::BufferPool::addToFreeBuffers(ByteBuffer* buffer) {
    std::lock_guard lock(mutex);
    if (freeBuffers.empty() || freeBuffers[0]->getTotalSize() >= buffer->getTotalSize()) {
        freeBuffers.push_back(buffer);
    } else {
        for (std::vector<bochan::ByteBuffer*>::iterator it = freeBuffers.begin(); it != freeBuffers.end(); ++it) {
            if ((*it)->getTotalSize() <= buffer->getTotalSize()) {
                freeBuffers.insert(it + 1, buffer);
                break;
            }
        }
    }
}

bool bochan::BufferPool::deleteBuffer(ByteBuffer* buffer, bool checkIfFree) {
    std::lock_guard lock(mutex);
    BOCHAN_DEBUG("Deleting buffer {:x} of size {} ({})", reinterpret_cast<uintptr_t>(buffer), buffer->getTotalByteSize(), checkIfFree);
    if (checkIfFree) {
        std::vector<bochan::ByteBuffer*>::iterator it = std::find(freeBuffers.begin(), freeBuffers.end(), buffer);
        if (it == freeBuffers.end()) {
            BOCHAN_WARN("Failed to remove buffer {:x} as it's being used!", reinterpret_cast<uintptr_t>(buffer));
            return false;
        }
        freeBuffers.erase(it);
    }
    byteSize -= buffer->getTotalSize();
    delete buffer;
    return true;
}
