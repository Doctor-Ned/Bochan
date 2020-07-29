#pragma once
#include "Thread.h"

#define THREAD_RESULT_SUCCESS 0UL
#define THREAD_RESULT_TERMINATED 1UL

namespace bochan {
    class BochanThread sealed : public Thread {
    public:
        BOCHANAPI ~BochanThread();
        BOCHANAPI bool run(ThreadFunc func, void* ptr) override;
        BOCHANAPI bool isRunning() override;
        BOCHANAPI void interrupt() override;
        BOCHANAPI bool isInterrupted() override;
        BOCHANAPI bool join() override;
        BOCHANAPI bool terminate() override;
        BOCHANAPI DWORD getThreadId();
    private:
        BOCHANAPI void closeThreadHandle();
        BOCHANAPI static DWORD threadProc(LPVOID param);
        std::atomic_bool interrupted{ false }, running{ false };
        std::mutex mutex;
        HANDLE threadHandle{ nullptr };
    };
}
