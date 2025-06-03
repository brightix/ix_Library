//
// Created by ix on 25-5-22.
//
#pragma once
#include <memory>
#include <libavutil/mem.h>
extern "C"{
    #include <libavformat/avformat.h>
}


struct AudioFormat
{
    int sample_rate;
    int channels;
    AVSampleFormat sample_fmt;
};

// struct AudioData
// {
//     static void av_samples_deleter(uint8_t** ptr)
//     {
//         if (ptr)
//         {
//             av_freep(&ptr[0]);
//             av_freep(&ptr);
//         }
//     }
//
//     std::unique_ptr<uint8_t*, decltype(&av_samples_deleter)> data;
//     uint32_t size;
//     double pts;
//
//     AudioData(uint8_t** raw_data, uint32_t size, double pts)
//         : data(raw_data, av_samples_deleter), size(size), pts(pts) {}
//
//     AudioData() : data(nullptr, av_samples_deleter), size(0), pts(0) {}
// };

struct AudioData {
    uint8_t* data;
    size_t size;
    int nb_samples;
    int channels;
    int sample_rate;
    AVSampleFormat format;
    double pts;
    uint32_t pos = 0;
    AudioData(uint8_t* data, size_t size, int samples, int ch, int rate, AVSampleFormat fmt, double pts)
        : data(data), size(size), nb_samples(samples), channels(ch), sample_rate(rate), format(fmt), pts(pts) {}

    ~AudioData() {
        if (data) av_freep(&data);
    }
};

// struct AudioData
// {
//     //std::unique_ptr<uint8_t*, decltype(&av_samples_deleter)> data;
//     uint8_t** data;
//     uint32_t size;
//     double pts;
//
//     AudioData(uint8_t** raw_data, uint32_t size, double pts)
//         : data(raw_data), size(size), pts(pts) {}
//
//     AudioData() : data(nullptr), size(0), pts(0) {}
//     ~AudioData()
//     {
//         if (data)
//         {
//             av_freep(&data[0]);
//             av_freep(&data);
//         }
//     }
// };
