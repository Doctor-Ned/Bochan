// BochanConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "BochanEncoder.h"

#include <iostream>

using namespace bochan;

int main() {
    BufferPool bufferPool(1024 * 1024 * 1024);
    BochanEncoder encoder(&bufferPool);
    encoder.initialize(BochanCodec::AAC, 48000, 64000);
    return 0;
}