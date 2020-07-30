#include "pch.h"
#include "BochanTCPServer.h"

bochan::BochanTCPServer::BochanTCPServer(BufferPool* bufferPool) : TCPServer(bufferPool) {}

bochan::BochanTCPServer::~BochanTCPServer() {
    if (isListening() || isConnected()) {
        shutdown();
        close();
    }
    WinsockUtil::wsaCleanup(this);
}

bool bochan::BochanTCPServer::bindAndListen(const char* ipAddress, unsigned short port) {
    BOCHAN_DEBUG("Attempting to bind a socket to {}:{}...", ipAddress, port);
    if (!WinsockUtil::wsaStartup(this)) {
        return false;
    }
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        BOCHAN_ERROR("Failed to create socket ({})!", WSAGetLastError());
        WinsockUtil::wsaCleanup(this);
        return false;
    }
    address = { AF_INET, port, inet_addr(ipAddress), {0} };
    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) != 0) {
        BOCHAN_ERROR("Failed to bind socket ({})!", WSAGetLastError());
        WinsockUtil::wsaCleanup(this);
        shutdown();
        close();
        return false;
    }
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        BOCHAN_ERROR("Failed to start listening ({})!", WSAGetLastError());
        WinsockUtil::wsaCleanup(this);
        shutdown();
        close();
        return false;
    }
    BOCHAN_DEBUG("Socket bound to {}:{} and listening!", inet_ntoa(address.sin_addr), address.sin_port);
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
        BOCHAN_DEBUG("Accepted connection from {}:{}!", inet_ntoa(addr.sin_addr), addr.sin_port);
    }
    shutdownServer();
    closeServer();
    return true;
}

bool bochan::BochanTCPServer::send(ByteBuffer* buff) {
    if (::send(clientSocket, reinterpret_cast<char*>(buff->getPointer()), buff->getUsedSize(), 0) == SOCKET_ERROR) {
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
        if (!::shutdown(serverSocket, SD_BOTH)) {
            BOCHAN_WARN("Failed to shut down server socket ({})!", WSAGetLastError());
            return false;
        }
    }
    return true;
}

bool bochan::BochanTCPServer::shutdownClient() {
    if (clientSocket != INVALID_SOCKET) {
        if (!::shutdown(clientSocket, SD_BOTH)) {
            BOCHAN_WARN("Failed to shut down client socket ({})!", WSAGetLastError());
            return false;
        }
    }
    return true;
}

bool bochan::BochanTCPServer::closeServer() {
    if (serverSocket != INVALID_SOCKET) {
        if (!::closesocket(serverSocket)) {
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
        if (!::closesocket(clientSocket)) {
            BOCHAN_WARN("Failed to close client socket ({})!", WSAGetLastError());
            return false;
        } else {
            clientSocket = INVALID_SOCKET;
        }
    }
    return true;
}
