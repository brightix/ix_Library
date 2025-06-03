//
// Created by ix on 25-5-19.
//
#pragma once

#include <string>

inline std::string get_time_string_from_second(int second)
{
    int h = second / 3600;
    int m = second % 3600 / 60;
    int s = second % 60;
    return (h >= 10 ? std::to_string(h) : "0" + std::to_string(h)) + ":" + (m >= 10 ? std::to_string(m) : "0" + std::to_string(m)) + ":" + (s >= 10 ? std::to_string(s) : "0" + std::to_string(s));
}

// inline SDL_Rect transform_video_rect(int win_w,int win_h, int width, int height)
// {
//     SDL_Rect res(0,0,0,0);
//     if (width > height) // 横屏视频
//     {
//         double scale = win_w * 1.0 / width;
//         res.w = win_w;
//         res.h = height * scale;
//     }
//     else
//     {
//         double scale = win_h * 1.0 / height;
//         res.w = width * scale;
//         res.h = win_h;
//     }
//     res.x = (win_w - res.w)/2;
//     res.y = (win_h - res.h)/2;
//     return res;
// }

inline SDL_Rect transform_video_rect(int w, int h, int dst_x, int dst_y, int dst_w, int dst_h)
{
    SDL_Rect res = {0,0,0,0};
    double scale_w = static_cast<double>(dst_w) / w;
    double scale_h = static_cast<double>(dst_h) / h;
    double scale = (scale_w < scale_h) ? scale_w : scale_h;  // 选最小缩放，保证视频能完整显示（保持比例）

    res.w = static_cast<int>(w * scale);
    res.h = static_cast<int>(h * scale);
    res.x = dst_x + (dst_w - res.w) / 2;
    res.y = dst_y + (dst_h - res.h) / 2;
    return res;
}

