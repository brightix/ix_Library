//
// Created by ix on 25-5-23.
//
#pragma once
#include "AudioData.h"
#include "../Enums/Enums.h"
struct VideoInfo
{
    bool is_audio_valid;
    AudioFormat audio_format;
    int width;
    int height;
    double fps;
    double start_pts;
    int32_t duration;
    int64_t duration_ms;
    DecodeType type;
    VideoInfo(AudioFormat a, int w, int h, double fps,double sp, int64_t d, int32_t d_ms, int64_t tf)
        : audio_format(a), width(w), height(h), fps(fps), start_pts(sp), duration(d), duration_ms(d_ms)
    {}
    VideoInfo(): audio_format(), width(0), height(0), fps(0), start_pts(0), duration(0)
    {
    }
};
