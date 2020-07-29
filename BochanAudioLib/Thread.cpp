#include "pch.h"
#include "Thread.h"

bochan::ThreadFunc bochan::Thread::getThreadFunc() {
    return threadFunc;
}

void* bochan::Thread::getPtr() {
    return ptr;
}
