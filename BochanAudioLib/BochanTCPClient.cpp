#include "pch.h"
#include "BochanTCPClient.h"

bochan::BochanTCPClient::BochanTCPClient(BufferPool& bufferPool) : TCPClient(bufferPool) {}

bochan::BochanTCPClient::~BochanTCPClient() {
    if (isConnected()) {
        shutdown();
        close();
    }
    WinsockUtil::wsaCleanup(this);
}

bool bochan::BochanTCPClient::connect(gsl::cstring_span ipAddress, unsigned short port) {
    if (ipAddress.empty()) {
        BOCHAN_ERROR("Provided IP address is empty!");
        return false;
    }
    BOCHAN_DEBUG("Attempting to connect to {}:{}...", ipAddress.cbegin(), port);
    if (!WinsockUtil::wsaStartup(this)) {
        return false;
    }
    shutdown();
    close();
    socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET) {
        BOCHAN_ERROR("Failed to create socket ({})!", WSAGetLastError());
        WinsockUtil::wsaCleanup(this);
        return false;
    }
    address = { AF_INET, htons(port), 0, {0} };
    if (int ret = InetPtonA(address.sin_family, ipAddress.cbegin(), &address.sin_addr); ret != 1) {
        if (ret == 0) {
            BOCHAN_ERROR("An invalid IP address was provided!");
        } else {
            BOCHAN_ERROR("Failed to convert the IP address ({})!", WSAGetLastError());
        }
        shutdown();
        close();
        WinsockUtil::wsaCleanup(this);
        return false;
    }
    if (::connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        BOCHAN_ERROR("Connection failed ({})!", WSAGetLastError());
        shutdown();
        close();
        WinsockUtil::wsaCleanup(this);
        return false;
    }
    BOCHAN_DEBUG("Connection established!");
    return true;
}

bool bochan::BochanTCPClient::send(gsl::not_null<ByteBuffer*> buff) {
    if (::send(socket, reinterpret_cast<char*>(buff->getPointer()), static_cast<int>(buff->getUsedSize()), 0) == SOCKET_ERROR) {
        BOCHAN_ERROR("Failed to send {} data ({})!", buff->getUsedSize(), WSAGetLastError());
        return false;
    }
    return true;
}

bochan::ByteBuffer* bochan::BochanTCPClient::receive() {
    int received = ::recv(socket, recvBuffer, TCP_SOCKET_BUFFER_SIZE, 0);
    if (received == 0) {
        return nullptr;
    }
    if (received == SOCKET_ERROR) {
        BOCHAN_ERROR("Failed to receive data ({})!", WSAGetLastError());
        return nullptr;
    }
    ByteBuffer* buff = bufferPool->getBuffer(received);
    memcpy(buff->getPointer(), recvBuffer, received);
    return buff;
}

bool bochan::BochanTCPClient::shutdown() {
    if (socket != INVALID_SOCKET) {
        if (::shutdown(socket, SD_BOTH) == SOCKET_ERROR) {
            BOCHAN_WARN("Failed to shut down socket ({})!", WSAGetLastError());
            return false;
        }
    }
    return true;
}

bool bochan::BochanTCPClient::close() {
    if (socket != INVALID_SOCKET) {
        if (::closesocket(socket) == SOCKET_ERROR) {
            BOCHAN_WARN("Failed to close socket ({})!", WSAGetLastError());
            return false;
        } else {
            socket = INVALID_SOCKET;
        }
    }
    return true;
}

bool bochan::BochanTCPClient::isConnected() {
    return socket != INVALID_SOCKET;
}
