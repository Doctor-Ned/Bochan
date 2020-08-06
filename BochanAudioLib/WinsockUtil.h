#pragma once

#include "pch.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

namespace bochan {
    class WinsockUtil sealed {
    public:
        WinsockUtil() = delete;
        BOCHANAPI static bool wsaStartup(void* owner);
        BOCHANAPI static bool wsaCleanup(void* owner);
    private:
        static std::vector<void*> wsaInvokers;
    };
}