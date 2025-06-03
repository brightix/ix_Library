#pragma once
#include <QObject>
#include <SDL_audio.h>
#include "../Enums/Enums.h"
#include "../AudioThread/AudioThread.h"
//#include "../RenderThread/RenderThread.h"
#include "VideoData.h"
#include "AudioData.h"
#include "VideoInfo.h"
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}


class RenderThread;







class DecoderWorker : public QObject {
    Q_OBJECT
public:
    void set_connect();

    DecoderWorker();

    bool get_decode_status();
    bool get_isSeek();
    double get_decode_current_time();
    VideoInfo get_video_info() const;

    void decode_one_frame_at(double tar, AudioThread *audio_thread, RenderThread *render_thread);

    void seek_by_offset(double current_pts, double offset_sec, RenderThread *render_thread, AudioThread *audio_thread);
    void seek_by_offset_video_only(double current_pts, double offset_sec, RenderThread *render_thread);

    void handle_finished(DecodeType type, AudioThread *audio_thread, RenderThread *render_thread);

    void wait_to_finished();



    //new
    std::shared_ptr<VideoData> change(AVCodecContext *codec_ctx, AVFrame *frame, double timestamp);

    std::shared_ptr<AudioData> audio_change(AVCodecContext *audio_codec_ctx, AVFrame *frame, double timestamp, SDL_AudioSpec *obtained_spec);

    double get_timestamp(const AVPacket *pkt, AVFrame *frame, const AVStream *av_stream);

    void set_decode_video_type(AVPixelFormat type);
    void set_decode_audio_type(AVSampleFormat type);

    void seek(double val);

    void set_seek_offset(double val);

    void set_seek_pos(double val);


    ~DecoderWorker();

    void pretreatment(DecodeType type, std::string path);

    bool pretreatment_video();

    bool pretreatment_audio();

private:

signals:
    void start_pretreatment(DecodeType type, std::string path);
    void finished();
    void format_ready(VideoInfo fmt);
    void init_window();
    void decode_ready(std::atomic<bool>* isStoped,AudioThread* audio_thread, RenderThread* render_thread);
    void decode_video_only_ready(std::atomic<bool>* isStoped,RenderThread* render_thread);
public slots:
private slots:
    void on_finished();


    //自定义槽函数
private:
    void decode(std::atomic<bool> *isStoped, AudioThread *audio_thread, RenderThread *render_thread);

    void decode_running(AVPacket *pkt, AVFrame *video_frame, AVFrame *audio_frame, RenderThread *render_thread,
                        AudioThread *audio_thread);

    void decode_seeking(AudioThread *audio_thread, RenderThread *render_thread);

    void decode_flushing(AVPacket *pkt, AVFrame *video_frame, AVFrame *audio_frame, AudioThread *audio_thread,
                         RenderThread *render_thread);

    void decode_finished(AudioThread *audio_thread, RenderThread *render_thread);

    void decode_stopped();

    void decode_video_only(std::atomic<bool> *isStoped, RenderThread *render_thread);

    void decode_audio_only(std::atomic<bool> *isStoped, AudioThread *audio_thread);

    void decode_video_with_audio(std::atomic<bool> *isStoped, AudioThread *audio_thread, RenderThread *render_thread);

private:
    QString m_current_path;
    AVFormatContext* fmt_ctx;
    SwsContext* sws_ctx;
//视频包
    AVCodecContext* video_codec_ctx;
    AVCodecContext* audio_codec_ctx;
    MemoryPool_shared<VideoData> video_data_pool;
    //int video_data_size;
//音频包
    MemoryPool_shared<AudioData> audio_data_pool;
    //std::unique_ptr<AudioData> audio_data;
    //int audio_data_size;
    SwrContext* swr_ctx;

    std::unordered_map<std::string,std::pair<bool,std::function<void()>>> equip_status;

    VideoInfo fmt;

    int video_stream_idx, audio_stream_idx;

    ////音频时钟
    //atomic<double> current

    //播放中切换视频格式
    std::atomic<AVPixelFormat> cur_decode_video_type;
    std::atomic<AVPixelFormat> next_decode_video_type;


    //播放中切换音频格式
    std::atomic<AVSampleFormat> cur_decode_audio_type;
    std::atomic<AVSampleFormat> next_decode_audio_type;

    //状态
    std::atomic<bool> isDecoding;
    std::atomic<bool> isSeek;
    std::atomic<double> decode_clock;
    std::mutex start_decode_mtx;

    std::mutex stop_entirely;
    std::condition_variable wait_to_clear;
    std::mutex wait_mtx;
    //快进
    std::atomic<double> is_preview;
    std::atomic<double> seek_offset;
    std::atomic<double> seek_pos;

    //状态机
    enum DecodeState
    {
        Running,
        Seeking,
        Flushing,
        Finished_audio_only,
        Finished_video_only,
        Finished_both,
        Finished,
        Stopped
    };
    std::atomic<DecodeState> state;

};