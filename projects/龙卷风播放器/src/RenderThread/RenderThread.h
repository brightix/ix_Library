//
// Created by ix on 25-5-21.
//
#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
#include <QThread>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <SDL.h>
#include <QWidget>
#include "../../src/ffmpeg/VideoData.h"
#include "../ffmpeg/DecoderWorker.h"
#include "../utils/MemoryPool/MemoryPool.h"
#include <QTimer>
#include "../utils/PauseElapsedTimer/PauseElpasedTimer.h"
class RenderThread : public QThread{
    Q_OBJECT
public:
    RenderThread():winId(nullptr), audio_clock(nullptr), isPaused(nullptr),
                    local_timer(nullptr)
    {
    }

    RenderThread(void *winId, std::atomic<double> *audio_clock, std::atomic<bool> *isPaused);

    ~RenderThread();

    size_t get_buff_size();
    double get_render_current_time();

    double get_render_localtime();

    bool is_finished();

    void clear_buffer();

    //设置
    void set_dst_w_h(int w, int h);

    void set_render_one_frame(bool val);

    void set_dst_rect(QRect rect);

    void set_window_resize(bool val);

    void add_video_frame(std::shared_ptr<VideoData> videoData);

public slots:


signals:
    void render_finished();

protected:
    void run() override;



    void load_default_bg();


private:
    void* winId;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;


    //MemoryPool<VideoData> video_frame_pool;
    std::deque<std::shared_ptr<VideoData>> video_frames;
    std::mutex frame_mutex;
    //RingBuffer<VideoData> video_frame_buffer;
    //当前帧
    std::shared_ptr<VideoData> frame_buf;


    //同步
    std::atomic<double>* audio_clock;
    std::atomic<double> render_clock;
    //暂停与播放
    std::atomic<bool>* isPaused;
    //渲染窗口大小
    std::atomic<int> dst_x,dst_y,dst_w,dst_h;

    std::atomic<bool> is_finished_;

    //无音频流的计时器
    PauseElapsedTimer* local_timer;
    std::atomic<double> localtime;

    //窗口拉伸
    std::atomic<bool> is_window_resize;
};



#endif //RENDERTHREAD_H
