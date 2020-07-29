#pragma once

#include "pch.h"

#define THREAD_JOIN_TIMEOUT 10000

namespace bochan {
    class BOCHANAPI Thread;
    typedef void (*ThreadFunc)(Thread*, void* ptr);

    class BOCHANAPI Thread {
    public:
        Thread() = default;
        virtual ~Thread() = default;
        virtual bool run(ThreadFunc func, void* ptr) = 0;
        virtual bool isRunning() = 0;
        virtual void interrupt() = 0;
        virtual bool isInterrupted() = 0;
        virtual bool join() = 0;
        virtual bool terminate() = 0;
        ThreadFunc getThreadFunc();
        void* getPtr();
    protected:
        ThreadFunc threadFunc{ nullptr };
        void* ptr{ nullptr };
    };
}