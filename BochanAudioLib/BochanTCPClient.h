#pragma once

#include "TCPClient.h"
#include "WinsockUtil.h"

namespace bochan {
    class BOCHANAPI BochanTCPClient sealed : public TCPClient {
    public:
        BochanTCPClient(BufferPool& bufferPool);
        ~BochanTCPClient();
        bool connect(const char* ipAddress, unsigned short port) override;
        bool send(ByteBuffer* buff) override;
        ByteBuffer* receive() override;
        bool shutdown() override;
        bool close() override;
        bool isConnected() override;
    private:
        SOCKET socket{ INVALID_SOCKET };
        sockaddr_in address{};
        char recvBuffer[TCP_SOCKET_BUFFER_SIZE]{ 0 };
    };
}
