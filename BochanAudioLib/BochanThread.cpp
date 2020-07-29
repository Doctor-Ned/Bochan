#include "pch.h"
#include "BochanThread.h"

#include <windows.h>

bochan::BochanThread::~BochanThread() {
    if (threadHandle) {
        if (running && !join()) {
            BOCHAN_WARN("Failed to join thread while destroying the object, terminating...");
            if (!terminate()) {
                BOCHAN_ERROR("Failed to terminate thread while destroying the object!");
            }
        }
        closeThreadHandle();
    }
}

bool bochan::BochanThread::run(ThreadFunc func) {
    std::lock_guard lock(mutex);
    if (running) {
        BOCHAN_WARN("Attempted to invoke run on a thread that is already running!");
        return false;
    }
    running = true;
    interrupted = false;
    this->threadFunc = func;
    threadHandle = CreateThread(nullptr, 0ULL, threadProc, this, 0UL, nullptr);
    if (!threadHandle) {
        BOCHAN_ERROR("Failed to create a thread ({})!", GetLastError());
        running = false;
        return false;
    }
    return true;
}

bool bochan::BochanThread::isRunning() {
    if (running) {
        DWORD exitCode;
        if (!GetExitCodeThread(threadHandle, &exitCode)) {
            BOCHAN_WARN("Failed to check thread exit code ({})!", GetLastError());
            return true;
        }
        if (exitCode == STILL_ACTIVE) {
            return true;
        }
        running = false;
    }
    return running;
}

void bochan::BochanThread::interrupt() {
    interrupted = true;
}

bool bochan::BochanThread::isInterrupted() {
    return interrupted;
}

bool bochan::BochanThread::join() {
    std::lock_guard lock(mutex);
    if (!running) {
        return true;
    }
    interrupt();
    DWORD waitResult = WaitForSingleObject(threadHandle, THREAD_JOIN_TIMEOUT);
    switch (waitResult) {
        case WAIT_TIMEOUT:
            BOCHAN_WARN("Thread join attempt timed out!");
            return false;
        case WAIT_FAILED:
            BOCHAN_ERROR("Thread join attempt failed ({})!", GetLastError());
            running = false;
            return false;
        case WAIT_ABANDONED:
            BOCHAN_WARN("Thread join attempt resulted in WAIT_ABANDONED!");
            [[fallthrough]];
        default:
            [[fallthrough]];
        case WAIT_OBJECT_0:
            running = false;
            closeThreadHandle();
            return true;
    }
}

bool bochan::BochanThread::terminate() {
    std::lock_guard lock(mutex);
    if (!running) {
        return true;
    }
    if (!TerminateThread(threadHandle, 1UL)) {
        BOCHAN_WARN("Thread termination attempt failed ({})!", GetLastError());
        return false;
    }
    running = false;
    closeThreadHandle();
    return true;
}

BOCHANAPI DWORD bochan::BochanThread::getThreadId() {
    return threadHandle ? GetThreadId(threadHandle) : 0UL;
}

void bochan::BochanThread::closeThreadHandle() {
    if (!CloseHandle(threadHandle)) {
        BOCHAN_WARN("Failed to close the thread handle!");
    } else {
        threadHandle = nullptr;
    }
}

DWORD WINAPI bochan::BochanThread::threadProc(LPVOID param) {
    Thread* thread = reinterpret_cast<Thread*>(param);
    ThreadFunc threadFunc = thread->getThreadFunc();
    if (threadFunc) {
        threadFunc(thread);
    } else {
        BOCHAN_WARN("An empty thread has been started!");
    }
    return 0UL;
}
