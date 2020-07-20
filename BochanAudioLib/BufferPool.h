#pragma once
#include "Buffer.h"

namespace bochan {
    class BufferPool sealed {
    public:
        BOCHANAPI BufferPool(size_t maxSize);
        BOCHANAPI Buffer* getBuffer(size_t size);
        BOCHANAPI bool freeBuffer(Buffer* buffer);
        BOCHANAPI bool freeAndRemoveBuffer(Buffer* buffer);
        BOCHANAPI void flushUnused();
        BOCHANAPI size_t getTotalSize() const;
        BOCHANAPI size_t getUsedSize() const;
        BOCHANAPI size_t getFreeSize() const;
        BOCHANAPI size_t getUnallocatedSize() const;
        BOCHANAPI size_t getMaxSize() const;
    private:
        BOCHANAPI bool freeBuffer(Buffer* buffer, bool remove);
        BOCHANAPI void addUsed(Buffer* buffer);
        BOCHANAPI Buffer* addBuffer(size_t size, bool setFree);
        BOCHANAPI void addToFreeBuffers(Buffer* buffer);
        BOCHANAPI bool deleteBuffer(Buffer* buffer, bool checkIfFree);
        mutable std::recursive_mutex mutex{};
        std::vector<Buffer*> usedBuffers{}, freeBuffers{};
        size_t maxSize{};
        size_t byteSize{ 0UL };
        size_t usedSize{ 0UL };
    };
}
