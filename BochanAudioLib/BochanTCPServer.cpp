#include "pch.h"
#include "BochanTCPServer.h"

bochan::BochanTCPServer::BochanTCPServer(BufferPool& bufferPool) : TCPServer(bufferPool) {}

bochan::BochanTCPServer::~BochanTCPServer() {
    if (isListening() || isConnected()) {
        shutdown();
        close();
    }
    WinsockUtil::wsaCleanup(this);
}

bool bochan::BochanTCPServer::bindAndListen(gsl::cstring_span ipAddress, unsigned short port) {
    if (ipAddress.empty()) {
        BOCHAN_ERROR("Provided IP address is empty!");
        return false;
    }
    BOCHAN_DEBUG("Attempting to bind a socket to {}:{}...", ipAddress.cbegin(), port);
    if (!WinsockUtil::wsaStartup(this)) {
        return false;
    }
    shutdown();
    close();
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
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
        shutdownServer();
        closeServer();
        WinsockUtil::wsaCleanup(this);
        return false;
    }
    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) != 0) {
        BOCHAN_ERROR("Failed to bind socket ({})!", WSAGetLastError());
        shutdownServer();
        closeServer();
        WinsockUtil::wsaCleanup(this);
        return false;
    }
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        BOCHAN_ERROR("Failed to start listening ({})!", WSAGetLastError());
        shutdownServer();
        closeServer();
        WinsockUtil::wsaCleanup(this);
        return false;
    }
    char buff[64]{ 0 };
    PCSTR str = InetNtopA(AF_INET, &address.sin_addr, buff, 64);
    if (str == nullptr) {
        BOCHAN_WARN("Unable to convert the address to string ({})!", WSAGetLastError());
        BOCHAN_DEBUG("Socket bound to and listening!");
    } else {
        BOCHAN_DEBUG("Socket bound to {}:{} and listening!", str, address.sin_port);
    }
    return true;
}

bool bochan::BochanTCPServer::isListening() {
    return serverSocket != INVALID_SOCKET;
}

bool bochan::BochanTCPServer::acceptClient() {
    clientSocket = accept(serverSocket, static_cast<sockaddr*>(nullptr), nullptr);
    if (clientSocket == INVALID_SOCKET) {
        BOCHAN_ERROR("Failed to accept a client connection ({})!", WSAGetLastError());
        return false;
    }
    sockaddr_in addr;
    int addrSize{ sizeof(addr) };
    if (getpeername(clientSocket, reinterpret_cast<sockaddr*>(&addr), &addrSize) == SOCKET_ERROR) {
        BOCHAN_WARN("Accepted connection, but getpeername failed ({})!", WSAGetLastError());
    } else {
        char buff[64]{ 0 };
        PCSTR str = InetNtopA(AF_INET, &address.sin_addr, buff, 64);
        if (str == nullptr) {
            BOCHAN_WARN("Unable to convert the address to string ({})!", WSAGetLastError());
            BOCHAN_DEBUG("Accepted connection!");
        } else {
            BOCHAN_DEBUG("Accepted connection from {}:{}!", str, addr.sin_port);
        }
    }
    closeServer();
    return true;
}

bool bochan::BochanTCPServer::send(gsl::not_null<ByteBuffer*> buff) {
    if (::send(clientSocket, reinterpret_cast<char*>(buff->getPointer()), static_cast<int>(buff->getUsedSize()), 0) == SOCKET_ERROR) {
        BOCHAN_ERROR("Failed to send {} data ({})!", buff->getUsedSize(), WSAGetLastError());
        return false;
    }
    return true;
}

bochan::ByteBuffer* bochan::BochanTCPServer::receive() {
    int received = ::recv(clientSocket, recvBuffer, TCP_SOCKET_BUFFER_SIZE, 0);
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

bool bochan::BochanTCPServer::shutdown() {
    return shutdownClient() && shutdownServer();
}

bool bochan::BochanTCPServer::close() {
    return closeClient() && closeServer();
}

bool bochan::BochanTCPServer::isConnected() {
    return clientSocket != INVALID_SOCKET;
}

bool bochan::BochanTCPServer::shutdownServer() {
    if (serverSocket != INVALID_SOCKET) {
        if (::shutdown(serverSocket, SD_BOTH) == SOCKET_ERROR) {
            BOCHAN_WARN("Failed to shut down server socket ({})!", WSAGetLastError());
            return false;
        }
    }
    return true;
}

bool bochan::BochanTCPServer::shutdownClient() {
    if (clientSocket != INVALID_SOCKET) {
        if (::shutdown(clientSocket, SD_BOTH) == SOCKET_ERROR) {
            BOCHAN_WARN("Failed to shut down client socket ({})!", WSAGetLastError());
            return false;
        }
    }
    return true;
}

bool bochan::BochanTCPServer::closeServer() {
    if (serverSocket != INVALID_SOCKET) {
        if (::closesocket(serverSocket) == SOCKET_ERROR) {
            BOCHAN_WARN("Failed to close server socket ({})!", WSAGetLastError());
            return false;
        } else {
            serverSocket = INVALID_SOCKET;
        }
    }
    return true;
}

bool bochan::BochanTCPServer::closeClient() {
    if (clientSocket != INVALID_SOCKET) {
        if (::closesocket(clientSocket) == SOCKET_ERROR) {
            BOCHAN_WARN("Failed to close client socket ({})!", WSAGetLastError());
            return false;
        } else {
            clientSocket = INVALID_SOCKET;
        }
    }
    return true;
}
