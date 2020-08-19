#pragma once
#include "Buffer.h"

namespace bochan {
    class BufferPool sealed {
    public:
        BOCHANAPI BufferPool(size_t maxSize);
        BOCHANAPI ~BufferPool();
        BufferPool(BufferPool&) = delete;
        BufferPool(BufferPool&&) = delete;
        BufferPool& operator=(BufferPool&) = delete;
        BufferPool& operator=(BufferPool&&) = delete;
        BOCHANAPI ByteBuffer* getBuffer(size_t size);
        BOCHANAPI bool freeBuffer(gsl::not_null<ByteBuffer*> buffer);
        BOCHANAPI bool freeAndRemoveBuffer(gsl::not_null<ByteBuffer*> buffer);
        BOCHANAPI void flushUnused();
        BOCHANAPI size_t getTotalSize() const;
        BOCHANAPI size_t getUsedSize() const;
        BOCHANAPI size_t getFreeSize() const;
        BOCHANAPI size_t getUnallocatedSize() const;
        BOCHANAPI size_t getMaxSize() const;
    private:
        BOCHANAPI bool freeBuffer(gsl::not_null<ByteBuffer*> buffer, bool remove);
        BOCHANAPI void addUsed(gsl::not_null<ByteBuffer*> buffer);
        BOCHANAPI ByteBuffer* addBuffer(size_t size, bool setFree);
        BOCHANAPI void addToFreeBuffers(gsl::not_null<ByteBuffer*> buffer);
        BOCHANAPI bool deleteBuffer(gsl::not_null<ByteBuffer*> buffer, bool checkIfFree);
        mutable std::recursive_mutex mutex{};
        std::vector<ByteBuffer*> usedBuffers{}, freeBuffers{};
        size_t maxSize{};
        size_t byteSize{ 0UL };
        size_t usedSize{ 0UL };
    };
}
