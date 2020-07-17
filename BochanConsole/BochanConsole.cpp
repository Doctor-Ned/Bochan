// BochanConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

int main() {
    AVFrame* frame = av_frame_alloc();
    av_frame_free(&frame);
    return 0;
}