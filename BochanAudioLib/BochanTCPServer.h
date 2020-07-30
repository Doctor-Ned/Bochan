#pragma once

#include "TCPServer.h"
#include "WinsockUtil.h"

namespace bochan {
    class BochanTCPServer sealed : public TCPServer {
    public:
        BochanTCPServer(BufferPool* bufferPool);
        ~BochanTCPServer();
        bool bindAndListen(const char* ipAddress, unsigned short port) override;
        bool isListening() override;
        bool acceptClient() override;
        bool send(ByteBuffer* buff) override;
        ByteBuffer* receive() override;
        bool shutdown() override;
        bool close() override;
        bool isConnected() override;
    private:
        bool shutdownServer();
        bool shutdownClient();
        bool closeServer();
        bool closeClient();
        SOCKET serverSocket{ INVALID_SOCKET }, clientSocket{ INVALID_SOCKET };
        sockaddr_in address{};
        char recvBuffer[TCP_SOCKET_BUFFER_SIZE]{ 0 };
    };
}
