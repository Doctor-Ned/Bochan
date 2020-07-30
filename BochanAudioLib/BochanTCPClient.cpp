#include "pch.h"
#include "BochanTCPClient.h"

bochan::BochanTCPClient::BochanTCPClient(BufferPool* bufferPool) : TCPClient(bufferPool) {}

bochan::BochanTCPClient::~BochanTCPClient() {
    if (isConnected()) {
        shutdown();
        close();
    }
    WinsockUtil::wsaCleanup(this);
}

bool bochan::BochanTCPClient::connect(const char* ipAddress, unsigned short port) {
    return false;
}

bool bochan::BochanTCPClient::send(ByteBuffer* buff) {
    return false;
}

bochan::ByteBuffer* bochan::BochanTCPClient::receive() {
    return nullptr;
}

bool bochan::BochanTCPClient::shutdown() {
    return false;
}

bool bochan::BochanTCPClient::close() {
    return false;
}

bool bochan::BochanTCPClient::isConnected() {
    return false;
}
