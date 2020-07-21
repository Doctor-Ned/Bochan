// BochanConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "BochanEncoder.h"
#include "BochanDecoder.h"
#include "CodecUtil.h"

#include <iostream>

using namespace bochan;

int main() {
    BufferPool bufferPool(1024 * 1024 * 1024);
    BochanEncoder encoder(&bufferPool);
    encoder.initialize(BochanCodec::Opus, 48000, 64000);
    BochanDecoder decoder(&bufferPool);
    decoder.initialize(BochanCodec::Opus, 48000, 64000);
    ByteBuffer* buff = bufferPool.getBuffer(encoder.getInputBufferByteSize());
    double t = 0;
    double tincr = 2.0 * M_PI * 440.0 / 48000.0;
    for (int i = 0; i < 200; ++i) {
        size_t buffPos = 0ULL;
        do {
            double value = sin(t) * 32000.0;
            t += tincr;
            int16_t intVal = static_cast<int16_t>(value);
            for (int j = 0; j < CodecUtil::CHANNELS; ++j) {
                memcpy(buff->getPointer() + buffPos, &intVal, sizeof(int16_t));
                buffPos += sizeof(int16_t);
            }
        } while (buffPos < buff->getSize());
        size_t inSize = buff->getSize(), midSize = 0ULL, outSize = 0ULL;
        std::vector<ByteBuffer*> inBuffs, outBuffs;
        inBuffs = encoder.encode(buff);
        for (ByteBuffer* inBuff : inBuffs) {
            midSize += inBuff->getByteSize();
            std::vector<ByteBuffer*> output = decoder.decode(inBuff);
            for (ByteBuffer* outBuff : output) {
                for (int j = 0; j < outBuff->getSize() / 2; ++j) {
                    BOCHAN_WARN("{}", *reinterpret_cast<int16_t*>(outBuff->getPointer() + j * 2));
                }
                outSize += outBuff->getByteSize();
                outBuffs.push_back(outBuff);
            }
        }
        BOCHAN_INFO("In: {} | Mid: {} | Out: {}", inSize, midSize, outSize);
        for (ByteBuffer* in : inBuffs) {
            bufferPool.freeBuffer(in);
        }
        for (ByteBuffer* out : outBuffs) {
            bufferPool.freeBuffer(out);
        }
    }
    return 0;
}