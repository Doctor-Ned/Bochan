#pragma once

#include "Buffer.h"

namespace bochan {
    struct AudioPacket {
        ByteBuffer* buffer{ nullptr };
        int64_t pts{ 0 };
    };
}
