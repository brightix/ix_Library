//
// Created by ix on 25-5-22.
//
#pragma once
extern "C"{
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
}


struct VideoData{
    uint8_t* buffer = nullptr;
    uint8_t* data[4] = {nullptr};
    int linesize[4] = {0};
    int width;
    int height;
    int pitch;
    double pts;
    VideoData(int width, int height, AVPixelFormat fmt, double pts)
        : width(width), height(height), pts(pts)
    {
        av_image_alloc(data,linesize,width,height,fmt,1);
        buffer = data[0];
        pitch = linesize[0];
    }
    VideoData(): width(0), height(0), pitch(0), pts(0)
    {
    }

    ~VideoData()
    {
        if (buffer)
        {
            av_freep(&data[0]);
        }
    }
};
