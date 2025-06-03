//
// Created by ix on 25-5-21.
//
#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H
#include <QThread>
#include <SDL_audio.h>

#include "../utils/RingBuffer/RingBuffer.h"
#include "../ffmpeg/AudioData.h"
#include "../utils/MemoryPool/MemoryPool.h"
#include "../ffmpeg/VideoInfo.h"
struct VideoInfo;

class AudioThread : public QThread{

    Q_OBJECT
public:

    AudioThread(VideoInfo fmt, std::atomic<bool> *isPaused);
    AudioThread(): isPaused(nullptr), audioDev(0), wanted_spec(), obtained_spec(), audio_fmt()
    {
    }

    ~AudioThread();

    std::atomic<double> *get_audio_clock();

    double get_audio_current_time();

    SDL_AudioSpec *get_audio_obtained_spec();

    void clear_buffer();

    bool is_finished();

    void set_clear_SDL_QueueAudio();

    void add_audio_frame(std::shared_ptr<AudioData> frame);
    //设置
    void set_current_decode_clock(double val);

    void audio_call_back(Uint8 *stream, int len);
public slots:
signals:
    void err_finished();
    void audio_finished();
protected:
    void run() override;

    double calc_audio_clock(double current_pts, AVSampleFormat sample_format);


private:
    //MemoryPool<AudioData> memory_pool;
    //同步
    std::atomic<double> audio_clock;
    std::atomic<double> current_decode_clock;
    std::atomic<bool>* isPaused;
    //强制退出，例如视频切换了
    std::atomic<bool> clear_SDL_QueueAudio;

    SDL_AudioDeviceID audioDev;
    SDL_AudioSpec wanted_spec, obtained_spec;
    VideoInfo video_info;

    //结束标识符
    std::atomic<bool> is_finished_;
public:
    std::atomic<uint64_t> played_samples;
    AudioFormat audio_fmt;
    std::shared_ptr<AudioData> audio_read_buf;
    std::unique_ptr<RingBuffer<std::shared_ptr<AudioData>>> audioRingBuffer;
};




#endif //AUDIOTHREAD_H
