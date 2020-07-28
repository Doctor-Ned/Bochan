// BochanConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "BochanEncoder.h"
#include "BochanDecoder.h"
#include "CodecUtil.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

using namespace bochan;

void bochanEncodeDecode() {
    const BochanCodec CODEC = BochanCodec::Opus;
    const int SAMPLE_RATE = 48000;
    const unsigned long long BIT_RATE = 64000;
    BufferPool bufferPool(1024 * 1024 * 1024);
    BochanEncoder encoder(&bufferPool);
    if (!encoder.initialize(CODEC, SAMPLE_RATE, BIT_RATE)) {
        BOCHAN_CRITICAL("Encoder initialization failed!");
        return;
    }
    BochanDecoder decoder(&bufferPool);
    if (!decoder.initialize(CODEC, SAMPLE_RATE, BIT_RATE, decoder.needsExtradata(CODEC) ? encoder.getExtradata() : nullptr)) {
        BOCHAN_CRITICAL("Decoder initialization failed!");
        return;
    }
    ByteBuffer* buff = bufferPool.getBuffer(encoder.getInputBufferByteSize());
    double t = 0;
    double tincr = 2.0 * M_PI * 440.0 / static_cast<double>(SAMPLE_RATE);
    FILE* outputFile, * inputFile;
    if (fopen_s(&outputFile, "output.dat", "w")) {
        BOCHAN_CRITICAL("Failed to open output file!");
        return;
    }
    if (fopen_s(&inputFile, "input.dat", "w")) {
        BOCHAN_CRITICAL("Failed to open input file!");
        return;
    }
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
        fwrite(buff->getPointer(), 1, buff->getByteSize(), inputFile);
        std::vector<ByteBuffer*> inBuffs, outBuffs;
        inBuffs = encoder.encode(buff);
        for (ByteBuffer* inBuff : inBuffs) {
            midSize += inBuff->getByteSize();
            std::vector<ByteBuffer*> output = decoder.decode(inBuff);
            for (ByteBuffer* outBuff : output) {
                /*for (int j = 0; j < outBuff->getSize() / 2; ++j) {
                    BOCHAN_WARN("{}", *reinterpret_cast<int16_t*>(outBuff->getPointer() + j * 2));
                }*/
                fwrite(outBuff->getPointer(), 1, outBuff->getByteSize(), outputFile);
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
    bufferPool.freeBuffer(buff);
    fclose(inputFile);
    fclose(outputFile);
}

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec* codec, enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat* p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec* codec) {
    return 48000;
    const int* p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}

/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec* codec) {
    return AV_CH_LAYOUT_STEREO;
    const uint64_t* p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels > best_nb_channels) {
            best_ch_layout = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

static void encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt,
                   FILE* output) {
    int ret;

    /* send the frame for encoding */
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending the frame to the encoder\n");
        exit(1);
    }

    /* read all the available output packets (in general there may be any
     * number of them */
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            exit(1);
        }

        fwrite(pkt->data, 1, pkt->size, output);
        av_packet_unref(pkt);
    }
}

static int get_format_from_sample_fmt(const char** fmt,
                                      enum AVSampleFormat sample_fmt) {
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char* fmt_be, * fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry* entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

static void decode(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame,
                   FILE* outfile) {
    int i, ch;
    int ret, data_size;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error submitting the packet to the decoder\n");
        exit(1);
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            /* This should not occur, checking just for paranoia */
            fprintf(stderr, "Failed to calculate data size\n");
            exit(1);
        }
        for (i = 0; i < frame->nb_samples; i++)
            for (ch = 0; ch < dec_ctx->channels; ch++)
                fwrite(frame->data[ch] + static_cast<size_t>(data_size) * i, 1, data_size, outfile);
    }
}

void avcodecEncode() {
    const char* filename;
    const AVCodec* codec;
    AVCodecContext* c = NULL;
    AVFrame* frame;
    AVPacket* pkt;
    int i, j, k, ret;
    FILE* f;
    uint16_t* samples;
    float t, tincr;

    filename = "avcodec_encoded.dat";

    /* find the encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 64000;

    /* check that the encoder supports s16 pcm input */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, c->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s",
                av_get_sample_fmt_name(c->sample_fmt));
        exit(1);
    }

    /* select other audio parameters supported by the encoder */
    c->sample_rate = select_sample_rate(codec);
    c->channel_layout = select_channel_layout(codec);
    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    if (fopen_s(&f, filename, "wb")) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    /* packet for holding encoded output */
    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "could not allocate the packet\n");
        exit(1);
    }

    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate audio frame\n");
        exit(1);
    }

    frame->nb_samples = c->frame_size;
    frame->format = c->sample_fmt;
    frame->channel_layout = c->channel_layout;

    /* allocate the data buffers */
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate audio data buffers\n");
        exit(1);
    }

    /* encode a single tone sound */
    t = 0;
    tincr = 2 * M_PI * 440.0 / c->sample_rate;
    for (i = 0; i < 200; i++) {
        /* make sure the frame is writable -- makes a copy if the encoder
         * kept a reference internally */
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1);
        samples = (uint16_t*)frame->data[0];

        for (j = 0; j < c->frame_size; j++) {
            samples[2 * j] = (int)(sin(t) * 10000);

            for (k = 1; k < c->channels; k++)
                samples[2 * j + k] = samples[2 * j];
            t += tincr;
        }
        encode(c, frame, pkt, f);
    }

    /* flush the encoder */
    encode(c, NULL, pkt, f);

    fclose(f);

    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&c);
}

void avcodecDecode() {
    const char* outfilename, * filename;
    const AVCodec* codec;
    AVCodecContext* c = NULL;
    AVCodecParserContext* parser = NULL;
    int len, ret;
    FILE* f, * outfile;
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t* data;
    size_t   data_size;
    AVPacket* pkt;
    AVFrame* decoded_frame = NULL;
    enum AVSampleFormat sfmt;
    int n_channels = 0;
    const char* fmt;

    filename = "avcodec_encoded.dat";
    outfilename = "avcodec_decoded.dat";

    pkt = av_packet_alloc();

    /* find the audio decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_MP2);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "Parser not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    if (fopen_s(&f, filename, "rb")) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    if (fopen_s(&outfile, outfilename, "wb")) {
        av_free(c);
        exit(1);
    }

    /* decode until eof */
    data = inbuf;
    data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);

    while (data_size > 0) {
        if (!decoded_frame) {
            if (!(decoded_frame = av_frame_alloc())) {
                fprintf(stderr, "Could not allocate audio frame\n");
                exit(1);
            }
        }

        ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                               data, data_size,
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            exit(1);
        }
        data += ret;
        data_size -= ret;

        if (pkt->size)
            decode(c, pkt, decoded_frame, outfile);

        if (data_size < AUDIO_REFILL_THRESH) {
            memmove(inbuf, data, data_size);
            data = inbuf;
            len = fread(data + data_size, 1,
                        AUDIO_INBUF_SIZE - data_size, f);
            if (len > 0)
                data_size += len;
        }
    }

    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    decode(c, pkt, decoded_frame, outfile);

    /* print output pcm infomations, because there have no metadata of pcm */
    sfmt = c->sample_fmt;

    if (av_sample_fmt_is_planar(sfmt)) {
        const char* packed = av_get_sample_fmt_name(sfmt);
        printf("Warning: the sample format the decoder produced is planar "
               "(%s). This example will output the first channel only.\n",
               packed ? packed : "?");
        sfmt = av_get_packed_sample_fmt(sfmt);
    }

    n_channels = c->channels;
    if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
        goto end;

    printf("Play the output audio file with the command:\n"
           "ffplay -f %s -ac %d -ar %d %s\n",
           fmt, n_channels, c->sample_rate,
           outfilename);
end:
    fclose(outfile);
    fclose(f);

    avcodec_free_context(&c);
    av_parser_close(parser);
    av_frame_free(&decoded_frame);
    av_packet_free(&pkt);
}

void avcodecEncodeDecodeTest() {
    avcodecEncode();
    avcodecDecode();
}

int main() {
    //avcodecEncodeDecodeTest();
    bochanEncodeDecode();
    return 0;
}