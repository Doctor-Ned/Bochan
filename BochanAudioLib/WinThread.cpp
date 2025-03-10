#include "pch.h"
#include "WinThread.h"

#include <windows.h>

bochan::WinThread::~WinThread() {
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

bool bochan::WinThread::run(ThreadFunc func, void* ptr) {
    std::lock_guard lock(mutex);
    if (running) {
        BOCHAN_WARN("Attempted to invoke run on a thread that is already running!");
        return false;
    }
    running = true;
    interrupted = false;
    this->threadFunc = func;
    this->ptr = ptr;
    if (threadHandle) {
        closeThreadHandle();
    }
    threadHandle = CreateThread(nullptr, 0ULL, threadProc, this, 0UL, nullptr);
    if (!threadHandle) {
        BOCHAN_ERROR("Failed to create a thread ({})!", GetLastError());
        running = false;
        return false;
    }
    return true;
}

bool bochan::WinThread::isRunning() {
    if (running) {
        DWORD exitCode;
        if (!GetExitCodeThread(threadHandle, &exitCode)) {
            BOCHAN_WARN("Failed to check thread exit code ({})!", GetLastError());
            return true;
        }
        switch (exitCode) {
            case STILL_ACTIVE:
                return true;
            case THREAD_RESULT_TERMINATED:
                BOCHAN_WARN("Detected thread termination exit code!");
                break;
            case THREAD_RESULT_SUCCESS:
                break;
            default:
                BOCHAN_WARN("Unrecognized thread exit code {}!", exitCode);
                break;
        }
        running = false;
    }
    return running;
}

void bochan::WinThread::interrupt() {
    interrupted = true;
}

bool bochan::WinThread::isInterrupted() {
    return interrupted;
}

bool bochan::WinThread::join() {
    std::lock_guard lock(mutex);
    if (!running) {
        closeThreadHandle();
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

bool bochan::WinThread::terminate() {
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

BOCHANAPI DWORD bochan::WinThread::getThreadId() {
    return threadHandle ? GetThreadId(threadHandle) : 0UL;
}

void bochan::WinThread::closeThreadHandle() {
    if (threadHandle) {
        if (!CloseHandle(threadHandle)) {
            BOCHAN_WARN("Failed to close the thread handle!");
        } else {
            threadHandle = nullptr;
        }
    }
}

DWORD WINAPI bochan::WinThread::threadProc(LPVOID param) {
    Thread* thread = reinterpret_cast<Thread*>(param);
    ThreadFunc threadFunc = thread->getThreadFunc();
    if (threadFunc) {
        threadFunc(thread, thread->getPtr());
    } else {
        BOCHAN_WARN("An empty thread has been started!");
    }
    return THREAD_RESULT_SUCCESS;
}
