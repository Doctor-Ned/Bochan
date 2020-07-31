#include "pch.h"
#include "TCPServer.h"

bochan::TCPServer::TCPServer(BufferPool& bufferPool) : TCPSocket(bufferPool) {}
