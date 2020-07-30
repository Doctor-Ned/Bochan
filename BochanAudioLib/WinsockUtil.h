#pragma once

#include "BochanLog.h"

#include <WinSock2.h>

namespace bochan {
    class BOCHANAPI WinsockUtil sealed {
    public:
        WinsockUtil() = delete;
        ~WinsockUtil() = delete;
        static bool wsaStartup(void* owner);
        static bool wsaCleanup(void* owner);
    private:
        static std::vector<void*> wsaInvokers;
    };
}