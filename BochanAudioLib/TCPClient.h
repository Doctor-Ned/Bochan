#pragma once

#include "TCPSocket.h"

namespace bochan {
    class BOCHANAPI TCPClient : public TCPSocket {
    public:
        TCPClient(BufferPool& bufferPool);
        ~TCPClient() = default;
        virtual bool connect(gsl::cstring_span ipAddress, unsigned short port) = 0; // blocking
    };
}