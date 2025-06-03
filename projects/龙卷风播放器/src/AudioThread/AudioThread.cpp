//
// Created by ix on 25-5-21.
//
#include "../../src/GlobalVariable.h"
#include "AudioThread.h"

#include <SDL_audio.h>
#include <QDebug>
#include <SDL.h>

#include "../ffmpeg/DecoderWorker.h"
using namespace std;

// extern condition_variable pause_cv;
// extern mutex pause_mtx;

static void AudioCallbackWrapper(void* userdata, Uint8* stream, int len)
{
    AudioThread* self = static_cast<AudioThread*>(userdata);
    self->audio_call_back(stream, len);
}
AudioThread::AudioThread(VideoInfo fmt,std::atomic<bool> *isPaused)
{
    // SDL_Init(SDL_INIT_AUDIO);
    this->isPaused = isPaused;
    wanted_spec.freq = fmt.audio_format.sample_rate;
    wanted_spec.format = AUDIO_S16SYS;  // SDL常用格式，可能需要转换
    wanted_spec.channels = fmt.audio_format.channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024;
    wanted_spec.callback = nullptr;//AudioCallbackWrapper;  // 先不设回调，用queue或者push方式
    wanted_spec.userdata = nullptr;
    audio_fmt = fmt.audio_format;
    video_info = fmt;
    audioDev = SDL_OpenAudioDevice(
        nullptr, 0,
        &wanted_spec, &obtained_spec,
        0
    );

    audio_fmt.sample_rate = obtained_spec.freq;
    audio_fmt.channels    = obtained_spec.channels;
    size_t size = fmt.audio_format.sample_rate / static_cast<double>(1024);
    audioRingBuffer = make_unique<RingBuffer<shared_ptr<AudioData>>>(size);
    is_finished_.store(false);
    clear_SDL_QueueAudio.store(false);
    played_samples.store(0);
}

AudioThread::~AudioThread()
{
    // requestInterruption();
    // quit();
    // wait();

    // 确保关闭音频设备
    if (audioDev) {
        SDL_CloseAudioDevice(audioDev);
        audioDev = 0;
    }
    qDebug() << "~AudioThread";
}

void AudioThread::run()
{
    if (audioDev == 0)
    {
        qWarning() << "SDL_OpenAudioDevice failed:" << SDL_GetError();
        emit err_finished();
        return;
    }

    SDL_PauseAudioDevice(audioDev,0);
    if (obtained_spec.format != AUDIO_S16SYS) {
        qWarning() << "Unsupported audio format:" << obtained_spec.format;
        SDL_CloseAudioDevice(audioDev);
        emit err_finished();
        return;
    }
    //QThread::msleep(50);
    while (true)
    {
        if (isPaused->load())
        {
            SDL_PauseAudioDevice(audioDev,1);
            unique_lock<mutex> lock(pause_mtx);
            pause_cv.wait(lock, [this]
            {
                return !isPaused->load();
            });
            SDL_PauseAudioDevice(audioDev,0);
        }
        if (clear_SDL_QueueAudio.load())
        {
            SDL_ClearQueuedAudio(audioDev);
            clear_SDL_QueueAudio.store(false);
        }
        if (!audioRingBuffer->read(audio_read_buf))
        {
            if (clear_SDL_QueueAudio.load())
            {
                SDL_ClearQueuedAudio(audioDev);
            }
            break;
        }
        audio_clock.store(calc_audio_clock(audio_read_buf->pts,audio_read_buf->format));
        SDL_QueueAudio(audioDev, audio_read_buf->data, audio_read_buf->size);
        uint32_t queued_bytes = SDL_GetQueuedAudioSize(audioDev);
        while (queued_bytes > 192000) {
            QThread::msleep(5);
            queued_bytes = SDL_GetQueuedAudioSize(audioDev);
            audio_clock.store(calc_audio_clock(audio_read_buf->pts,audio_read_buf->format));
        }
        //qDebug() << "audioThread（curPlayTime）:  " + QString::number(audio_clock.load());
    }
    uint32_t queued_bytes = SDL_GetQueuedAudioSize(audioDev);
    while (queued_bytes > 0)
    {
        queued_bytes = SDL_GetQueuedAudioSize(audioDev);
        audio_clock.store(calc_audio_clock(audio_read_buf->pts,audio_read_buf->format));
        //qDebug() << "audioThread（curPlayTime）:  " + QString::number(audio_clock.load());
    }
    is_finished_.store(true);
    qDebug() << "audio_thread_finished";
}

//
// void AudioThread::run()
// {
//     if (audioDev == 0)
//     {
//         qWarning() << "SDL_OpenAudioDevice failed:" << SDL_GetError();
//         emit err_finished();
//         return;
//     }
//
//     SDL_PauseAudioDevice(audioDev,0);
//     if (obtained_spec.format != AUDIO_S16SYS) {
//         qWarning() << "Unsupported audio format:" << obtained_spec.format;
//         SDL_CloseAudioDevice(audioDev);
//         emit err_finished();
//         return;
//     }
//     while (audio_clock.load() < video_info.duration)
//     {
//         if (isPaused->load())
//         {
//             SDL_PauseAudioDevice(audioDev,1);
//             unique_lock<mutex> lock(pause_mtx);
//             pause_cv.wait(lock, [this]
//             {
//                 return !isPaused->load();
//             });
//             SDL_PauseAudioDevice(audioDev,0);
//         }
//         uint32_t queued_bytes = SDL_GetQueuedAudioSize(audioDev);
//         while (queued_bytes > 192000) {
//             QThread::msleep(5);
//             queued_bytes = SDL_GetQueuedAudioSize(audioDev);
//             audio_clock.store(calc_audio_clock());
//         }
//         audio_clock.store(calc_audio_clock());
//         // qDebug() << "sample_fmt added:" << video_info.audio_format.sample_fmt
//         //      << "total_played:" << played_samples.load()
//         //      << "clock:" << calc_audio_clock()
//         //      << "channels:" << audio_fmt.channels
//         //      << "sample_rate:" << audio_fmt.sample_rate;
//         QThread::msleep(10);
//     }
//
//     is_finished_.store(true);
//     qDebug() << "audio_thread_finished";
// }

double AudioThread::calc_audio_clock(double current_pts,AVSampleFormat sample_format)
{
    Uint32 queued = SDL_GetQueuedAudioSize(audioDev);
    double byte_per_sample = av_get_bytes_per_sample(sample_format);
    double queued_time = queued / (audio_fmt.sample_rate * audio_fmt.channels * byte_per_sample);
    double res = current_pts - queued_time;
    // qDebug() << "audio pts: " << current_pts;
    // qDebug() << "queued: " << queued << ", byte_per_sample: " << byte_per_sample << ", queued_time: " << queued_time;
    // qDebug() << "calc_audio_clock: " << res;
    return res;
    // return double(played_samples.load()) / audio_fmt.sample_rate;
}

void AudioThread::audio_call_back(Uint8* stream, int len) {
    memset(stream, 0, len);  // 如果没数据，播放静音

    int bytes_per_sample = SDL_AUDIO_BITSIZE(obtained_spec.format) / 8;
    int total_written = 0;

    while (total_written < len) {
        if (!audio_read_buf || audio_read_buf->pos >= audio_read_buf->size) {
            if (!audioRingBuffer->read(audio_read_buf)) {
                break;  // 没有更多数据了，静音
            }
        }

        int bytes_left = audio_read_buf->size - audio_read_buf->pos;
        int bytes_to_copy = std::min(len - total_written, bytes_left);
        memcpy(stream + total_written,
               audio_read_buf->data + audio_read_buf->pos,
               bytes_to_copy);

        audio_read_buf->pos += bytes_to_copy;
        total_written += bytes_to_copy;

        // 更新时钟：样本 = 字节 / 通道数 / 每样本字节数
        int samples = bytes_to_copy / (audio_fmt.channels * bytes_per_sample);
        played_samples.fetch_add(samples);
    }
}


void AudioThread::add_audio_frame(std::shared_ptr<AudioData> frame)
{
    if (frame)
    {
        audioRingBuffer->write(move(frame));
    }
    else
    {
        audioRingBuffer->stop();
    }
}

void AudioThread::set_current_decode_clock(double val)
{
    current_decode_clock.store(val);
}

atomic<double> *AudioThread::get_audio_clock()
{
    return &audio_clock;
}

double AudioThread::get_audio_current_time()
{
    return audio_clock.load();
}

SDL_AudioSpec *AudioThread::get_audio_obtained_spec()
{
    return &obtained_spec;
}

void AudioThread::clear_buffer()
{
    audioRingBuffer->clear();
}

bool AudioThread::is_finished()
{
    return is_finished_.load();
}

void AudioThread::set_clear_SDL_QueueAudio()
{
    clear_SDL_QueueAudio.store(true);
}
