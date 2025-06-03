//
// Created by ix on 25-5-20.
//
#ifndef ENUMS_H
#define ENUMS_H
extern "C" {
#include <libavformat/avformat.h>
}
#include "magic_enum.hpp"
enum class VideoType : int
{
    RGB32 = AV_PIX_FMT_RGBA,
    YUV420P = AV_PIX_FMT_YUV420P,
    YUV422P = AV_PIX_FMT_YUV422P,
    YUV444P = AV_PIX_FMT_YUV444P,
    GRAY8 = AV_PIX_FMT_GRAY8,
    RGB24 = AV_PIX_FMT_RGB24,
    None = -1
};
enum class AudioType : int
{
    S16 = AV_SAMPLE_FMT_S16,
    None = -1
};

enum class DecodeType
{
    VideoWithAudio,
    Video,
    Audio,
    UNKNOWN
};

namespace Enums
{
template<typename T>
const std::string_view to_string(T t)
{
    return magic_enum::enum_name(t);
}
}




#endif //ENUMS_H
