//
// Created by ix on 25-5-21.
//
#include "../../src/GlobalVariable.h"
#include "RenderThread.h"

#include <iostream>

#include "../utils/formula.h"
#include <QDebug>

using namespace std;

// extern condition_variable pause_cv;
// extern mutex pause_mtx;

RenderThread::RenderThread(void* winId, atomic<double>* audio_clock, atomic<bool>* isPaused)
{
    this->winId = winId;
    this->audio_clock = audio_clock;
    this->isPaused = isPaused;
    //this->isStoped = isStoped;
    dst_w.store(0),
    dst_h.store(0);

    //render_one_frame.store(false);
    is_finished_.store(false);
    render_clock.store(0.0);
    is_window_resize.store(false);
    local_timer = audio_clock ? nullptr : new PauseElapsedTimer;
    localtime.store(0.0);
}


RenderThread::~RenderThread()
{
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (local_timer) delete local_timer;
    if (isRunning())
    {
        qDebug() << "还在跑";
    }
    if (isFinished())
    {
        qDebug() << "还没结束";
    }
    qDebug() << "~RenderThread" << this;
}

void RenderThread::run()
{
    if (!window) {
        window = SDL_CreateWindowFrom(winId);
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    if (local_timer) {
        local_timer->start();
        qDebug() << "刚启动计时器，elapsed:" << local_timer->elapsed(); // 应该大于 0
    }
    while (true)
    {
        // 更新当前时间
        double now_clock = 0.0;
        if (local_timer)
        {
            now_clock = local_timer->elapsed() / 1000.0;
        }
        else
        {
            now_clock = audio_clock->load();
        }
        localtime.store(now_clock);

        if (video_frames.empty())
        {
            QThread::msleep(1);
            continue;
        }

        // 控制暂停
        if (isPaused->load()) {
            if (local_timer) local_timer->pause();
            std::unique_lock<std::mutex> lock(pause_mtx);
            pause_cv.wait(lock, [this]{ return !isPaused->load(); });
            if (local_timer) local_timer->resume();
        }

        std::lock_guard<std::mutex> lock(frame_mutex);

        if (video_frames.empty())
        {
            continue; // 再次检查，避免空队列
        }
        if (video_frames.front() == nullptr)
        {
            break; // 播放结束信号
        }

        double pts = video_frames.front()->pts;
        render_clock.store(pts);
        //qDebug() << "pts " << pts;
        //qDebug() << "now_clock " << now_clock;
        double delay = pts - now_clock;
        //qDebug() << "delay " << delay;
        if (delay > 0.01)
        {
            // 时间还没到，稍后再播放
            continue;
        }
        else if (delay < -0.1)
        {
            // 丢帧
            qDebug() << "丢帧";
            video_frames.pop_front();
            continue;
        }

        frame_buf = video_frames.front();
        video_frames.pop_front();

        if (!texture)
        {
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                        SDL_TEXTUREACCESS_STREAMING, frame_buf->width, frame_buf->height);
        }

        if (SDL_UpdateTexture(texture, nullptr, frame_buf->data[0], frame_buf->pitch) != 0)
        {
            qDebug() << "SDL_UpdateTexture failed:" << SDL_GetError();
        }
        if (is_window_resize.load())
        {
            SDL_SetWindowSize(window,dst_w.load(),dst_h.load());
            is_window_resize.store(false);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // 黑底
        SDL_RenderClear(renderer);
        //auto dst = transform_video_rect(dst_w.load(), dst_h.load(), frame_buf->width, frame_buf->height);
        auto dst = transform_video_rect(frame_buf->width, frame_buf->height,dst_x.load(), dst_y.load(), dst_w.load(), dst_h.load());
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_RenderPresent(renderer);

        QThread::msleep(1); // 保障 CPU 友好
    }

    qDebug() << "视频放完了";
    is_finished_.store(true);
}

void RenderThread::add_video_frame(shared_ptr<VideoData> videoData)
{
    std::lock_guard<std::mutex> lock(frame_mutex);
    video_frames.push_back(std::move(videoData));
}

void RenderThread::set_dst_w_h(int w, int h)
{
    dst_w.store(w);
    dst_h.store(h);
}

void RenderThread::set_render_one_frame(bool val)
{
    //render_one_frame.store(val);
}

void RenderThread::set_dst_rect(QRect rect)
{
    dst_x.store(rect.x());
    dst_y.store(rect.y());
    dst_w.store(rect.width());
    dst_h.store(rect.height());
}

void RenderThread::set_window_resize(bool val)
{
    is_window_resize.store(val);
}

void RenderThread::load_default_bg()
{
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}


size_t RenderThread::get_buff_size()
{
    return video_frames.size();
}

bool RenderThread::is_finished()
{
    return is_finished_.load();
}
void RenderThread::clear_buffer()
{
    lock_guard lock(frame_mutex);
    video_frames.clear();
}

double RenderThread::get_render_current_time()
{
    return render_clock.load();
}
double RenderThread::get_render_localtime()
{
    return localtime.load();
}
