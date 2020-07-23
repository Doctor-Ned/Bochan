#include "pch.h"
#include "AudioCoder.h"
#include "CodecUtil.h"

bochan::AudioCoder::AudioCoder(BufferPool* bufferPool) : bufferPool(bufferPool) {}

bochan::ByteBuffer* bochan::AudioCoder::int16ToFloat(ByteBuffer* samples) {
    ByteBuffer* result = bufferPool->getBuffer(samples->getSize() * sizeof(float) / sizeof(int16_t));
    CodecUtil::int16ToFloat(samples, reinterpret_cast<float*>(result->getPointer()));
    return result;
}

bochan::ByteBuffer* bochan::AudioCoder::floatToInt16(ByteBuffer* fltp) {
    ByteBuffer* result = bufferPool->getBuffer(fltp->getSize() * sizeof(int16_t) / sizeof(float));
    CodecUtil::floatToInt16(fltp, reinterpret_cast<int16_t*>(result->getPointer()));
    return result;
}
