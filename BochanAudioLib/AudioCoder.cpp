#include "pch.h"
#include "AudioCoder.h"

bochan::AudioCoder::AudioCoder(BufferPool* bufferPool) : bufferPool(bufferPool) {}

bochan::ByteBuffer* bochan::AudioCoder::int16ToFloat(ByteBuffer* samples) {
    ByteBuffer* result = bufferPool->getBuffer(samples->getSize() * sizeof(float) / sizeof(int16_t));
    int16ToFloat(samples, reinterpret_cast<float*>(result->getPointer()));
    return result;
}

bochan::ByteBuffer* bochan::AudioCoder::floatToInt16(ByteBuffer* fltp) {
    ByteBuffer* result = bufferPool->getBuffer(fltp->getSize() * sizeof(int16_t) / sizeof(float));
    floatToInt16(fltp, reinterpret_cast<int16_t*>(result->getPointer()));
    return result;
}

void bochan::AudioCoder::int16ToFloat(ByteBuffer* from, float* to) {
    int16ToFloat(reinterpret_cast<int16_t*>(from->getPointer()), from->getSize() / 2ULL, to);
}

void bochan::AudioCoder::floatToInt16(ByteBuffer* from, int16_t* to) {
    floatToInt16(reinterpret_cast<float*>(from->getPointer()), from->getSize() / sizeof(float), to);
}

void bochan::AudioCoder::int16ToFloat(int16_t* from, size_t count, float* to) {
    for (int i = 0; i < count; ++i) {
        to[i] = static_cast<int16_t>(static_cast<float>(from[i]) / 32768.0f);
    }
}

void bochan::AudioCoder::floatToInt16(float* from, size_t count, int16_t* to) {
    for (int i = 0; i < count; ++i) {
        to[i] = static_cast<int16_t>(from[i] * 32767.9f);
    }
}
