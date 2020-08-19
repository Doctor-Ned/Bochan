#pragma once

#include "TCPClient.h"
#include "WinsockUtil.h"

namespace bochan {
    class BOCHANAPI BochanTCPClient sealed : public TCPClient {
    public:
        BochanTCPClient(BufferPool& bufferPool);
        ~BochanTCPClient();
        BochanTCPClient(BochanTCPClient&) = delete;
        BochanTCPClient(BochanTCPClient&&) = delete;
        BochanTCPClient& operator=(BochanTCPClient&) = delete;
        BochanTCPClient& operator=(BochanTCPClient&&) = delete;
        bool connect(gsl::not_null<const char*> ipAddress, unsigned short port) override;
        bool send(gsl::not_null<ByteBuffer*> buff) override;
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
