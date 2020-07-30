#pragma once

#include "TCPSocket.h"

namespace bochan {
    class BOCHANAPI TCPServer : public TCPSocket {
    public:
        TCPServer(BufferPool* bufferPool);
        ~TCPServer() = default;
        virtual bool bindAndListen(const char* ipAddress, unsigned short port) = 0;
        virtual bool isListening() = 0;
        virtual bool acceptClient() = 0; // non-blocking
    private:

    };
}
