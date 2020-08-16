// BochanConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "BochanEncoder.h"
#include "BochanDecoder.h"
#include "CodecUtil.h"
#include "SoundUtil.h"
#include "SignalProvider.h"
#include "AudioDevicePlayer.h"
#include "AudioFileProvider.h"
#include "AudioDeviceProvider.h"

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
    std::vector<const char*> DRIVERS = SoundUtil::getAudioDrivers();
    AudioDevicePlayer player{};
    if (!player.initializeDefault(nullptr, CONFIG.sampleRate)) {
        return;
    }
    AudioFileProvider provider;
    if (!provider.initialize("spanish_flea.flac", CONFIG.sampleRate, CodecUtil::getBytesPerSecond(CONFIG.sampleRate))) {
        return;
    }
    //AudioDeviceProvider provider;
    //if (!provider.initialize("Focusrite USB (Focusrite USB Au", CONFIG.sampleRate, CodecUtil::getBytesPerSecond(CONFIG.sampleRate), true)) {
    //    return;
    //}
    //if (!provider.record()) {
    //    BOCHAN_ERROR("Failed to start recording!");
    //    return;
    //}

    //SignalProvider provider(bufferPool);
    //if (!provider.initialize(CONFIG.sampleRate)) {
    //    return;
    //}
    //provider.setAmplitude(0.5);
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
    const int SECONDS = 5;
    const bool PLAY_DIRECT = false;
    size_t in{ 0 }, mid{ 0 }, out{ 0 };
    int iterations = static_cast<int>(SECONDS * CodecUtil::getBytesPerSecond(CONFIG.sampleRate) / encoder.getInputBufferByteSize());
    for (int i = 0; i < iterations; ++i) {
        if (!provider.fillBuffer(sampleBuff)) {
            continue;
        }
        in += sampleBuff->getUsedSize();
        if (PLAY_DIRECT) {
            player.queueData(sampleBuff);
        } else {
            std::vector<AudioPacket> inBuffs = encoder.encode(sampleBuff);
            for (AudioPacket packet : inBuffs) {
                mid += packet.buffer->getUsedSize();
                std::vector<ByteBuffer*> output = decoder.decode(packet);
                bufferPool.freeBuffer(packet.buffer);
                for (ByteBuffer* outBuff : output) {
                    out += outBuff->getUsedSize();
                    player.queueData(outBuff);
                    bufferPool.freeBuffer(outBuff);
                }
            }
        }
        if (!player.isPlaying()) {
            player.play();
        }
    }
    BOCHAN_INFO("IN: {} | MID: {} | OUT: {}", in, mid, out);
    player.stop();
    bufferPool.freeBuffer(sampleBuff);
}

int main() {
    CodecUtil::initialiseAvLog();
    bochanProviderPlayer();
    return 0;
}