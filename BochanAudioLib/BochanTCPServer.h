#pragma once

#include "TCPServer.h"
#include "WinsockUtil.h"

namespace bochan {
    class BOCHANAPI BochanTCPServer sealed : public TCPServer {
    public:
        BochanTCPServer(BufferPool& bufferPool);
        ~BochanTCPServer();
        BochanTCPServer(BochanTCPServer&) = delete;
        BochanTCPServer(BochanTCPServer&&) = delete;
        BochanTCPServer& operator=(BochanTCPServer&) = delete;
        BochanTCPServer& operator=(BochanTCPServer&&) = delete;
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
