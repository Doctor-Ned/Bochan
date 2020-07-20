#include "pch.h"
#include "CodecUtil.h"
#include "BochanEncoder.h"

bochan::BochanEncoder::BochanEncoder() {}

bochan::BochanEncoder::~BochanEncoder() {}

bool bochan::BochanEncoder::initialize(BochanCodec bochanCodec) {
    codecId = CodecUtil::getCodecId(bochanCodec);
    if (codecId == AV_CODEC_ID_NONE) {
        return false;
    }
    codec = avcodec_find_encoder(codecId);
    if (!codec) {
        return false;
    }
    return true;
}

void bochan::BochanEncoder::deinitialize() {
    codecId = AV_CODEC_ID_NONE;
}
