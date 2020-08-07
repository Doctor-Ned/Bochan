// BochanConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "BochanEncoder.h"
#include "BochanDecoder.h"
#include "CodecUtil.h"
#include "SignalProvider.h"
#include "BochanAudioPlayer.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
}

#undef main

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

using namespace bochan;

void bochanProviderPlayer() {
    const CodecConfig CONFIG{ BochanCodec::Opus, 48000, 64000 };
    BufferPool bufferPool(1024 * 1024 * 1024);
    SignalProvider provider(bufferPool);
    BochanAudioPlayer player{};
    if (!player.initializeDefault(nullptr, CONFIG.sampleRate)) {
        return;
    }
    if (!provider.initialize(CONFIG.sampleRate)) {
        return;
    }
    provider.setAmplitude(0.5);
    BochanEncoder encoder(bufferPool);
    if (!encoder.initialize(CONFIG)) {
        BOCHAN_CRITICAL("Encoder initialization failed!");
        return;
    }
    BochanDecoder decoder(bufferPool);
    if (!decoder.initialize(CONFIG, true, decoder.needsExtradata(CONFIG.codec) ? encoder.getExtradata() : nullptr)) {
        BOCHAN_CRITICAL("Decoder initialization failed!");
        return;
    }
    ByteBuffer* sampleBuff{ bufferPool.getBuffer(encoder.getInputBufferByteSize()) };
    const int SECONDS = 3;
    const bool PLAY_DIRECT = false;
    int iterations = static_cast<int>(SECONDS * player.getBytesPerSecond() / encoder.getInputBufferByteSize());
    for (int i = 0; i < iterations; ++i) {
        provider.fillBuffer(sampleBuff);
        if (PLAY_DIRECT) {
            player.queueData(sampleBuff);
        } else {
            std::vector<AudioPacket> inBuffs = encoder.encode(sampleBuff);
            for (AudioPacket packet : inBuffs) {
                std::vector<ByteBuffer*> output = decoder.decode(packet);
                bufferPool.freeBuffer(packet.buffer);
                for (ByteBuffer* outBuff : output) {
                    player.queueData(outBuff);
                    bufferPool.freeBuffer(outBuff);
                }
            }
        }
        if (!player.isPlaying()) {
            if (player.play()) {
                BOCHAN_INFO("Playback started!");
            }
        }
    }
    player.stop();
    bufferPool.freeBuffer(sampleBuff);
}

int main() {
    CodecUtil::initialiseAvLog();
    bochanProviderPlayer();
    return 0;
}