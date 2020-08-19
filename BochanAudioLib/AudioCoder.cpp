#include "pch.h"
#include "AudioCoder.h"
#include "CodecUtil.h"

bochan::AudioCoder::AudioCoder(BufferPool& bufferPool) : bufferPool(&bufferPool) {}

bochan::CodecConfig bochan::AudioCoder::getCodecConfig() const {
    return config;
}

bochan::ByteBuffer* bochan::AudioCoder::int16ToFloat(gsl::not_null<ByteBuffer*> samples) {
    ByteBuffer* result = bufferPool->getBuffer(samples->getUsedSize() * sizeof(float) / sizeof(int16_t));
    CodecUtil::int16ToFloat(samples, reinterpret_cast<float*>(result->getPointer()));
    return result;
}

bochan::ByteBuffer* bochan::AudioCoder::floatToInt16(gsl::not_null<ByteBuffer*> fltp) {
    ByteBuffer* result = bufferPool->getBuffer(fltp->getUsedSize() * sizeof(int16_t) / sizeof(float));
    CodecUtil::floatToInt16(fltp, reinterpret_cast<int16_t*>(result->getPointer()));
    return result;
}
