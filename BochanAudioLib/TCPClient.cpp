#include "pch.h"
#include "TCPClient.h"

bochan::TCPClient::TCPClient(BufferPool& bufferPool) : TCPSocket(bufferPool) {}
