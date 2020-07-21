#include "pch.h"
#include "AudioCoder.h"

bochan::AudioCoder::AudioCoder(BufferPool* bufferPool) : bufferPool(bufferPool) {}

bochan::ByteBuffer* bochan::AudioCoder::samplesToFloat(ByteBuffer* samples) {
    ByteBuffer* result = bufferPool->getBuffer(samples->getSize() * sizeof(float) / sizeof(int16_t));
    samplesToFloat(samples, reinterpret_cast<float*>(result->getPointer()));
    return result;
}

bochan::ByteBuffer* bochan::AudioCoder::floatToSamples(ByteBuffer* fltp) {
    ByteBuffer* result = bufferPool->getBuffer(fltp->getSize() * sizeof(int16_t) / sizeof(float));
    floatToSamples(fltp, reinterpret_cast<int16_t*>(result->getPointer()));
    return result;
}

void bochan::AudioCoder::samplesToFloat(ByteBuffer* from, float* to) {
    int16_t* int16Buff = reinterpret_cast<int16_t*>(from->getPointer());
    size_t sampleCount = from->getSize() / sizeof(int16_t);
    for (int i = 0; i < sampleCount; ++i) {
        to[i] = static_cast<float>(int16Buff[i]) / 32768.0f;
    }
}

void bochan::AudioCoder::floatToSamples(ByteBuffer* from, int16_t* to) {
    float* floatBuff = reinterpret_cast<float*>(from->getPointer());
    size_t sampleCount = from->getSize() / sizeof(float);
    for (int i = 0; i < sampleCount; ++i) {
        to[i] = static_cast<int16_t>(floatBuff[i] * 32767.9f);
    }
}
