#include "DecoderWorker.h"

#include <filesystem>
#include <QDebug>
#include <ranges>

#include "../utils/MemoryPool/MemoryPool.h"
#include "../RenderThread/RenderThread.h"
using namespace std;

void DecoderWorker::set_connect()
{
    //connect(this, &DecoderWorker::finished, this, &DecoderWorker::on_finished);
    connect(this, &DecoderWorker::start_pretreatment, this, &DecoderWorker::pretreatment);
    connect(this, &DecoderWorker::decode_ready, this, &DecoderWorker::decode);
    //connect(this, &DecoderWorker::decode_video_only_ready, this, &DecoderWorker::decode_video_only);

}

DecoderWorker::DecoderWorker()
{

    swr_ctx = nullptr;
    sws_ctx = nullptr;
    equip_status = {
        {
            "_", {
                true, [&]() {
                    static int times = 0;
                    qDebug() << times++;
                    if (fmt_ctx)
                    {
                        avformat_close_input(&fmt_ctx); // 关键：关闭输入流
                        avformat_free_context(fmt_ctx);
                        fmt_ctx = nullptr;
                    }

                    if (video_codec_ctx)
                    {
                        avcodec_free_context(&video_codec_ctx);
                        video_codec_ctx = nullptr;
                    }
                    if (audio_codec_ctx)
                    {
                        avcodec_free_context(&audio_codec_ctx);
                        audio_codec_ctx = nullptr;
                    }
                    if (sws_ctx) {
                        sws_freeContext(sws_ctx);  // 会自动设为nullptr
                        sws_ctx = nullptr;
                    }
                    if (swr_ctx) {
                        swr_free(&swr_ctx);  // 会自动设为nullptr
                    }
                    video_stream_idx = -1;
                    audio_stream_idx = -1;
                    fmt = VideoInfo{};
                }
            }
        },
    };
    video_stream_idx = -1;
    audio_stream_idx = -1;

    isDecoding.store(false);
    is_preview.store(-1);
    decode_clock.store(0.0);
    seek_offset.store(0.0);
    seek_pos.store(0.0);


    fmt_ctx = nullptr;
    video_codec_ctx = nullptr;
    audio_codec_ctx = nullptr;

    cur_decode_video_type.store(AV_PIX_FMT_NONE);
    next_decode_video_type.store(AV_PIX_FMT_RGBA);

    cur_decode_audio_type.store(AV_SAMPLE_FMT_NONE);
    next_decode_audio_type.store(AV_SAMPLE_FMT_S16);
    set_connect();
}

DecoderWorker::~DecoderWorker()
{
    on_finished();
}

void DecoderWorker::pretreatment(DecodeType type, string path)
{
    on_finished();

    fmt_ctx = avformat_alloc_context();
    int ret = avformat_open_input(&fmt_ctx,path.data(),nullptr,nullptr);
    if (ret < 0) {
        qDebug() << "打开文件失败：" << QString::fromStdString(path);
        return;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        qDebug() << "找 stream info 失败！";
        return;
    }
    if (type == DecodeType::UNKNOWN)
    {
        type = DecodeType::VideoWithAudio;
    }
    fmt.type = type;
    switch (type)
    {
    case DecodeType::VideoWithAudio:
    {
        bool has_audio = pretreatment_audio();
        bool has_video = pretreatment_video();
        if (has_audio && has_video)
        {
            break;
        }
        if (has_audio)
        {
            fmt.type = DecodeType::Audio;
        }
        else
        {
            fmt.type = DecodeType::Video;
        }
        break;
    }
    case DecodeType::Audio:
        if (!pretreatment_audio()) return;
        break;
    case DecodeType::Video:
        if (!pretreatment_video()) return;
        break;
    default: break;
    }

    fmt.duration_ms = fmt_ctx->duration;
    fmt.duration = fmt.duration_ms/1000000;
    emit format_ready(fmt);
}
bool DecoderWorker::pretreatment_video()
{
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }
    if (video_stream_idx == -1)
    {
        qDebug() << "video_stream_idx = -1";
        return false;
    }
    if (!fmt_ctx)
    {
        qDebug() << "fmt_ctx有问题";
    }

    if (!fmt_ctx->streams[video_stream_idx]) {
        qDebug() << "fmt_ctx->streams[" << video_stream_idx << "] 是 nullptr";
        return false;
    }
    AVCodecParameters* video_codec_par = fmt_ctx->streams[video_stream_idx]->codecpar;
    const AVCodec* decoder = avcodec_find_decoder(video_codec_par->codec_id);

    video_codec_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(video_codec_ctx,video_codec_par);
    avcodec_open2(video_codec_ctx,decoder,nullptr);

    fmt.width = video_codec_par->width;
    fmt.height = video_codec_par->height;

    AVStream* videoStream = fmt_ctx->streams[video_stream_idx];

    fmt.start_pts = videoStream->start_time * av_q2d(videoStream->time_base);
    fmt.fps = av_q2d(videoStream->avg_frame_rate);
    //fmt.duration = videoStream->duration * av_q2d(videoStream->time_base);
    return true;
}
bool DecoderWorker::pretreatment_audio()
{
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }

    if (audio_stream_idx == -1)
    {
        qDebug() << "audio_stream_idx == -1";
        return false;
    }
    AVCodecParameters* audio_par = fmt_ctx->streams[audio_stream_idx]->codecpar;
    const AVCodec* audio_decoder = avcodec_find_decoder(audio_par->codec_id);

    if (!audio_decoder) {qDebug() << "找不到对应的音频解码器"; return false; }

    audio_codec_ctx = avcodec_alloc_context3(audio_decoder);

    if (!audio_codec_ctx) { qDebug() << "无法分配音频 codec context"; return false; }

    if (avcodec_parameters_to_context(audio_codec_ctx, audio_par) < 0) { qDebug() << "无法将 codec 参数拷贝到 context"; return false; }

    // 5. 打开音频解码器
    if (avcodec_open2(audio_codec_ctx, audio_decoder, nullptr) < 0) { qDebug() << "打开音频解码器失败"; return false; }

    fmt.audio_format.sample_rate = audio_codec_ctx->sample_rate;
    fmt.audio_format.channels = audio_codec_ctx->channels;
    fmt.audio_format.sample_fmt = audio_codec_ctx->sample_fmt;
    return true;
}
void DecoderWorker::decode(atomic<bool>* isStoped, AudioThread* audio_thread, RenderThread* render_thread)
{

    //double pts;
    isDecoding.store(true);
    state.store(Running);
    switch (fmt.type)
    {
    case DecodeType::VideoWithAudio:
        decode_video_with_audio(isStoped,audio_thread,render_thread);
        break;
    case DecodeType::Video:
        decode_video_only(isStoped,render_thread);
        break;
    case DecodeType::Audio:
        decode_audio_only(isStoped,audio_thread);
        break;
    default: break;
    }


}

void DecoderWorker::decode_running(AVPacket* pkt, AVFrame* audio_frame, AVFrame* video_frame, RenderThread* render_thread, AudioThread* audio_thread)
{
    if (av_read_frame(fmt_ctx,pkt) >= 0)
    {
        if (pkt->stream_index == audio_stream_idx)
        {
            avcodec_send_packet(audio_codec_ctx,pkt);
            while (avcodec_receive_frame(audio_codec_ctx,audio_frame) >= 0)
            {
                AVStream* stream = fmt_ctx->streams[audio_stream_idx];
                double timestamp = get_timestamp(pkt,audio_frame, stream);

                audio_thread->add_audio_frame(audio_change(audio_codec_ctx,audio_frame, timestamp,audio_thread->get_audio_obtained_spec()));
                audio_thread->set_current_decode_clock(timestamp);
                //decode_clock.store(timestamp);
                // qDebug() << "audio_frame";
                // qDebug() << audio_frame->pts;
                //pts = timestamp;
            }
        }
        else if (pkt->stream_index == video_stream_idx)
        {
            avcodec_send_packet(video_codec_ctx,pkt);
            while (avcodec_receive_frame(video_codec_ctx,video_frame) >= 0)
            {
                AVStream* stream = fmt_ctx->streams[video_stream_idx];
                double timestamp = get_timestamp(pkt,video_frame, stream);
                render_thread->add_video_frame(change(
                    video_codec_ctx,
                    video_frame,
                    timestamp
                    ));
                decode_clock.store(timestamp);
            }
        }
        av_packet_unref(pkt);
    }
    else
    {
        state.store(Flushing);
    }
}
void DecoderWorker::decode_seeking(AudioThread* audio_thread, RenderThread* render_thread)
{
    double seek_pts = seek_offset.exchange(0.0);
    double current_pts = audio_thread->get_audio_current_time();
    seek_by_offset(current_pts,seek_pts,render_thread,audio_thread);
    state.store(Running);
}
void DecoderWorker::decode_flushing(AVPacket* pkt, AVFrame* audio_frame, AVFrame* video_frame, AudioThread* audio_thread, RenderThread* render_thread)
{
    //冲洗
    avcodec_send_packet(video_codec_ctx, nullptr);
    bool video_done = false, audio_done = false;
    while (!video_done)
    {
        int ret = avcodec_receive_frame(video_codec_ctx, video_frame);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        {
            video_done = true;
            break;
        }
        else if (ret >= 0)
        {
            AVStream* stream = fmt_ctx->streams[video_stream_idx];
            render_thread->add_video_frame(change(
                        video_codec_ctx,
                        video_frame,
                        get_timestamp(pkt,video_frame, stream)));
        }
    }
    avcodec_send_packet(audio_codec_ctx, nullptr);
    while (!audio_done)
    {
        int ret = avcodec_receive_frame(audio_codec_ctx, audio_frame);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        {
            audio_done = true;
            break;
        }
        else if (ret >= 0)
        {
            AVStream* stream = fmt_ctx->streams[audio_stream_idx];
            audio_thread->add_audio_frame(audio_change(
                audio_codec_ctx,
                audio_frame,
                get_timestamp(pkt,audio_frame, stream),
                audio_thread->get_audio_obtained_spec()));
        }

    }
    if (audio_done && video_done)
    {
        //handle_finished(render_thread,audio_thread);
        if (audio_thread)
        {
            audio_thread->add_audio_frame(nullptr);
        }
        if (render_thread)
        {
            render_thread->add_video_frame(nullptr);
        }
        state.store(Finished);
    }
}
void DecoderWorker::decode_finished(AudioThread* audio_thread, RenderThread* render_thread)
{
    while (audio_thread && !audio_thread->is_finished())
    {
        QThread::msleep(1);
    }
    if (render_thread)
    {
        render_thread->clear_buffer();
        while (!render_thread->is_finished())
        {
            QThread::msleep(1);
        }
    }
    state.store(Stopped);
}
void DecoderWorker::decode_stopped()
{
    qDebug() << "解码线程检测到音频播放完毕";
    //on_finished();
    isDecoding.store(false);
    wait_to_clear.notify_one();
    emit finished();
}

shared_ptr<VideoData> DecoderWorker::change(AVCodecContext *codec_ctx, AVFrame *frame, double timestamp)
{
    const int &w = codec_ctx->width;
    const int &h = codec_ctx->height;

    auto next_type = next_decode_video_type.load();
    auto cur_type = cur_decode_video_type.load();
    if (next_type != cur_type || !sws_ctx)
    {

        if (sws_ctx)
        {
            sws_freeContext(sws_ctx);
        }
        sws_ctx = sws_getContext(
            w, h, codec_ctx->pix_fmt,
            w, h, next_type,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
        cur_decode_video_type.store(next_type);
    }

    auto data = video_data_pool.get(w, h, next_type, timestamp);
    //auto data = make_shared<VideoData>(w,h,video_type,timestamp);
    sws_scale(sws_ctx,
              frame->data, frame->linesize,
              0, frame->height,
              data->data, data->linesize);
    return data;
}

shared_ptr<AudioData> DecoderWorker::audio_change(AVCodecContext* audio_codec_ctx, AVFrame* frame,double timestamp,SDL_AudioSpec* obtained_spec)
{
    auto audio_type = next_decode_audio_type.load();
    int target_sample_rate = obtained_spec->freq;
    int target_channels = obtained_spec->channels;
    int64_t out_ch_layout = (target_channels == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;

    if (cur_decode_audio_type.load() != audio_type || !swr_ctx)
    {
        if (swr_ctx)
        {
            swr_free(&swr_ctx);
        }
        int64_t in_ch_layout = audio_codec_ctx->channel_layout
                      ? audio_codec_ctx->channel_layout
                      : av_get_default_channel_layout(audio_codec_ctx->channels);
        swr_ctx = swr_alloc_set_opts(
            nullptr,
            out_ch_layout,       // 目标声道布局
            audio_type,                // 目标格式
            target_sample_rate,        // 目标采样率
            in_ch_layout, // 输入声道布局
            audio_codec_ctx->sample_fmt,     // 输入采样格式
            audio_codec_ctx->sample_rate,    // 输入采样率
            0, nullptr);
        if (!swr_ctx || swr_init(swr_ctx) < 0) {
            if (swr_ctx) swr_free(&swr_ctx);
            return nullptr;
        }
        cur_decode_audio_type.store(audio_type);
    }
    int dst_nb_samples = av_rescale_rnd(
        swr_get_delay(swr_ctx,audio_codec_ctx->sample_rate) + frame->nb_samples,
        target_sample_rate,
        audio_codec_ctx->sample_rate,
        AV_ROUND_UP
        );

    int out_linesize;
    uint8_t* out_buffer = nullptr;
    av_samples_alloc
    (
        &out_buffer,
        &out_linesize,
        target_channels,
        dst_nb_samples,
        audio_type,
        0
    );

    int converted = swr_convert(
        swr_ctx,
        &out_buffer,
        dst_nb_samples,
        (const uint8_t**)frame->extended_data,
        frame->nb_samples);
    if (converted < 0){ av_freep(&out_buffer); return nullptr; }

    //buff大小
    int actual_size = av_samples_get_buffer_size(
        nullptr,
        target_channels,
        converted,
        audio_type,
        1); // 1 表示不对齐，通常够用
    return audio_data_pool.get(out_buffer,actual_size,converted,target_channels,target_sample_rate,audio_type,timestamp);
}

double DecoderWorker::get_timestamp(const AVPacket* pkt, AVFrame* frame ,const AVStream* av_stream)
{
    double timestamp;
    if (frame->pts != AV_NOPTS_VALUE)
    {
        timestamp = frame->pts * av_q2d(av_stream->time_base);
    } else if (pkt->pts != AV_NOPTS_VALUE)
    {
        timestamp = pkt->pts * av_q2d(av_stream->time_base);
    } else
    {
        timestamp = -1;
    }
    return timestamp;
}

void DecoderWorker::on_finished()
{
    //释放资源
    for (auto &[_,e]: equip_status)
    {
        if (e.first)
        {
            e.second();
        }
    }
}

void DecoderWorker::set_decode_video_type(const AVPixelFormat type)
{
    next_decode_video_type.store(type);
}
void DecoderWorker::set_decode_audio_type(const AVSampleFormat type)
{
    next_decode_audio_type.store(type);
}
void DecoderWorker::seek(double val)
{
    is_preview.store(val);
}
void DecoderWorker::set_seek_offset(double val)
{
    seek_offset.store(val);
}

void DecoderWorker::set_seek_pos(double val)
{
    seek_pos.store(val);
}

bool DecoderWorker::get_decode_status()
{
    return isDecoding.load();
}
bool DecoderWorker::get_isSeek()
{
    return seek_offset.load() > 0 || seek_pos.load() > 0;
}
double DecoderWorker::get_decode_current_time()
{
    return decode_clock.load();
}
VideoInfo DecoderWorker::get_video_info() const
{
    return fmt;
}

//废案
    void DecoderWorker::decode_one_frame_at(double tar, AudioThread* audio_thread, RenderThread* render_thread) {

    double tar_sec = tar * fmt.duration;
    auto vs_time_base = fmt_ctx->streams[video_stream_idx]->time_base;
    int64_t pts = av_rescale_q((int64_t)(tar_sec * AV_TIME_BASE), AVRational{1, AV_TIME_BASE}, vs_time_base);
    av_seek_frame(fmt_ctx,video_stream_idx, pts,AVSEEK_FLAG_BACKWARD);
    //qDebug() << pts;

    avcodec_flush_buffers(video_codec_ctx);
    avcodec_flush_buffers(audio_codec_ctx);

    audio_thread->clear_buffer();
    render_thread->clear_buffer();
    //render_thread->set_render_one_frame(true);

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_idx) {
            int ret = avcodec_send_packet(video_codec_ctx, pkt);
            if (ret >= 0) {
                ret = avcodec_receive_frame(video_codec_ctx, frame);
                if (ret == 0) {
                    // 成功解出一帧，渲染它
                    render_thread->add_video_frame(change(
                    video_codec_ctx,
                    frame,
                    get_timestamp(pkt,frame, fmt_ctx->streams[video_stream_idx])));
                    break;
                }
            }
        }
        av_packet_unref(pkt);
    }

    //render_thread->set_render_one_frame(false);
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

void DecoderWorker::seek_by_offset(double current_pts, double offset_sec, RenderThread* render_thread, AudioThread* audio_thread)
{

    if (fmt.type == DecodeType::Audio)
    {
        return;
    }
    double target_sec = std::clamp(current_pts + offset_sec, 0.0, (double)fmt.duration);

    int64_t seek_target = (int64_t)(target_sec * AV_TIME_BASE);
    int ret = avformat_seek_file(fmt_ctx, -1, INT64_MIN, seek_target, INT64_MAX, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        qDebug() << "avformat_seek_file failed:" << ret;
    }
    AVRational tb = fmt_ctx->streams[video_stream_idx]->time_base;
    //int64_t seek_pts = av_rescale_q((int64_t)(target_sec * AV_TIME_BASE), AVRational{1, AV_TIME_BASE}, tb);

    //av_seek_frame(fmt_ctx, video_stream_idx, seek_pts, AVSEEK_FLAG_BACKWARD);
    qDebug() << "original pts:" << current_pts;
    qDebug() << "after pts:" << target_sec;

    qDebug() << "Seek to PTS:" << (int64_t)(target_sec * AV_TIME_BASE) * av_q2d(tb);

    // 清除缓存，只在用户主动seek时执行
    avcodec_flush_buffers(video_codec_ctx);
    avcodec_flush_buffers(audio_codec_ctx);

    render_thread->clear_buffer();
    audio_thread->clear_buffer();
    audio_thread->set_clear_SDL_QueueAudio();
    //audio_thread->set_current_decode_clock(target_sec);
}

void DecoderWorker::handle_finished(DecodeType type, AudioThread* audio_thread, RenderThread* render_thread)
{
    qDebug() << "解码完了";
    switch (type)
    {
    case DecodeType::VideoWithAudio:
        audio_thread->add_audio_frame(nullptr);
        while (!audio_thread->is_finished())
        {
            QThread::msleep(1);
        }
        render_thread->clear_buffer();
        render_thread->add_video_frame(nullptr);
        while (!render_thread->is_finished())
        {
            QThread::msleep(1);
        }
        break;
    case DecodeType::Audio:
        audio_thread->add_audio_frame(nullptr);
        while (!audio_thread->is_finished())
        {
            QThread::msleep(1);
        }
        break;
    case DecodeType::Video:
        render_thread->add_video_frame(nullptr);
        while (!render_thread->is_finished())
        {
            QThread::msleep(1);
        }
        isDecoding.store(false);
        wait_to_clear.notify_one();
        emit finished();
        break;
    default: break;
    }
    qDebug() << "解码线程检测到音频播放完毕";
    //on_finished();
    isDecoding.store(false);
    wait_to_clear.notify_one();
    emit finished();
}

void DecoderWorker::wait_to_finished()
{
    unique_lock lock(wait_mtx);
    wait_to_clear.wait(lock,[this]
    {
        return !isDecoding.load();
    });
    qDebug() << "总算是解码finished了";
}

void DecoderWorker::seek_by_offset_video_only(double current_pts, double offset_sec, RenderThread* render_thread)
{
    double target_sec = std::clamp(current_pts + offset_sec, 0.0, (double)fmt.duration);
    int flags = AVSEEK_FLAG_ANY;
    if (offset_sec < 0)
        flags |= AVSEEK_FLAG_BACKWARD;
    int64_t seek_target = (int64_t)(target_sec * AV_TIME_BASE);
    int ret = avformat_seek_file(fmt_ctx, -1, INT64_MIN, seek_target, INT64_MAX, flags);
    if (ret < 0) {
        qDebug() << "avformat_seek_file failed:" << ret;
    }
    AVRational tb = fmt_ctx->streams[video_stream_idx]->time_base;


    qDebug() << "original pts:" << current_pts;
    qDebug() << "after pts:" << target_sec;

    qDebug() << "Seek to PTS:" << (int64_t)(target_sec * AV_TIME_BASE) * av_q2d(tb);

    // 清除缓存，只在用户主动seek时执行
    avcodec_flush_buffers(video_codec_ctx);

    render_thread->clear_buffer();
}

//解码
void DecoderWorker::decode_video_only(atomic<bool>* isStoped,RenderThread* render_thread)
{
    AVPacket* pkt = av_packet_alloc();
    AVFrame* video_frame = av_frame_alloc();
    double pts;
    isDecoding.store(true);
    while (av_read_frame(fmt_ctx,pkt) >= 0)
    {
        // if (is_preview.load() > 0)
        // {
        //     decode_one_frame_at(is_preview.load(), audio_thread, render_thread);
        //     continue;
        // }
        if (isStoped->load())
        {
            render_thread->clear_buffer();
            handle_finished(fmt.type, nullptr,render_thread);
            return;
        }
        if (seek_offset.load() != 0)
        {
            double seek_pts = seek_offset.exchange(0.0);
            double current_pts = render_thread->get_render_current_time();
            seek_by_offset_video_only(current_pts,seek_pts,render_thread);
            continue;
        }
        if (pkt->stream_index == video_stream_idx)
        {
            avcodec_send_packet(video_codec_ctx,pkt);
            while (avcodec_receive_frame(video_codec_ctx,video_frame) >= 0)
            {
                AVStream* stream = fmt_ctx->streams[video_stream_idx];
                double timestamp = get_timestamp(pkt,video_frame, stream);
                render_thread->add_video_frame(change(
                    video_codec_ctx,
                    video_frame,
                    timestamp
                    ));
                decode_clock.store(timestamp);
            }
        }
        av_packet_unref(pkt);

    }


    //冲洗
    avcodec_send_packet(video_codec_ctx, nullptr);
    while (avcodec_receive_frame(video_codec_ctx, video_frame) >= 0)
    {
        AVStream* stream = fmt_ctx->streams[video_stream_idx];
        render_thread->add_video_frame(change(
                    video_codec_ctx,
                    video_frame,
                    get_timestamp(pkt,video_frame, stream)));
    }
    qDebug() << pts;
    handle_finished(fmt.type, nullptr,render_thread);
    av_packet_free(&pkt);
    av_frame_free(&video_frame);
}

void DecoderWorker::decode_audio_only(atomic<bool>* isStoped, AudioThread* audio_thread)
{
    AVPacket* pkt = av_packet_alloc();
    AVFrame* audio_frame = av_frame_alloc();
    double pts;
    isDecoding.store(true);
    while (av_read_frame(fmt_ctx,pkt) >= 0)
    {
        if (isStoped->load())
        {
            audio_thread->clear_buffer();
            handle_finished(fmt.type, audio_thread,nullptr);
            return;
        }
        if (seek_offset.load() != 0)
        {
            double seek_pts = seek_offset.exchange(0.0);
            double current_pts = audio_thread->get_audio_current_time();
            seek_by_offset(current_pts,seek_pts, nullptr, audio_thread);
            continue;
        }
        if (pkt->stream_index == audio_stream_idx)
        {
            avcodec_send_packet(audio_codec_ctx,pkt);
            while (avcodec_receive_frame(audio_codec_ctx,audio_frame) >= 0)
            {
                AVStream* stream = fmt_ctx->streams[audio_stream_idx];
                double timestamp = get_timestamp(pkt,audio_frame, stream);
                audio_thread->add_audio_frame(audio_change(audio_codec_ctx,audio_frame, timestamp,audio_thread->get_audio_obtained_spec()));
                audio_thread->set_current_decode_clock(timestamp);
            }
        }
        av_packet_unref(pkt);

    }


    //冲洗
    avcodec_send_packet(audio_codec_ctx, nullptr);
    while (avcodec_receive_frame(audio_codec_ctx, audio_frame) >= 0)
    {
        AVStream* stream = fmt_ctx->streams[audio_stream_idx];
        double timestamp = get_timestamp(pkt,audio_frame, stream);
        audio_thread->add_audio_frame(audio_change(audio_codec_ctx,audio_frame, timestamp,audio_thread->get_audio_obtained_spec()));
        audio_thread->set_current_decode_clock(timestamp);
    }
    qDebug() << pts;
    handle_finished(fmt.type, audio_thread, nullptr);
    av_packet_free(&pkt);
    av_frame_free(&audio_frame);
}

void DecoderWorker::decode_video_with_audio(atomic<bool>* isStoped, AudioThread* audio_thread, RenderThread* render_thread)
{
    AVPacket* pkt = av_packet_alloc();
    AVFrame* audio_frame = av_frame_alloc();
    AVFrame* video_frame = av_frame_alloc();
    bool running = true;
    while (running)
    {
        if (isStoped->load())
        {
            if (render_thread)
            {
                render_thread->clear_buffer();
                render_thread->add_video_frame(nullptr);
            }
            if (audio_thread)
            {
                audio_thread->clear_buffer();
                audio_thread->add_audio_frame(nullptr);
                audio_thread->set_clear_SDL_QueueAudio();
            }
            state.store(Finished);
        }
        if (seek_offset.load() != 0.0)
        {
            state.store(Seeking);
        }

        switch (state)
        {
        case Running:
            decode_running(pkt, audio_frame, video_frame, render_thread, audio_thread);
            break;
        case Seeking:
            decode_seeking(audio_thread,render_thread);
            break;
        case Flushing:
            decode_flushing(pkt, audio_frame, video_frame, audio_thread, render_thread);
            break;
        case Finished:
            //decode_finished(audio_thread, render_thread);
            handle_finished(fmt.type, audio_thread, render_thread);
            running = false;
            break;
        default: break;
        }
    }
    av_packet_free(&pkt);
    av_frame_free(&audio_frame);
    av_frame_free(&video_frame);
}



// optional<VideoInfo> DecoderWorker::pretreatment(string path)
// {
//     unique_lock lock(stop_entirely);
//     is_stop_entirely.wait(lock,[&]()
//     {
//         return !isDecoding.load();
//     });
//
//     on_finished();
//     fmt_ctx = avformat_alloc_context();
//     int ret = avformat_open_input(&fmt_ctx,path.data(),nullptr,nullptr);
//     if (ret < 0) {
//         qDebug() << "打开文件失败：" << QString::fromStdString(path);
//         return nullopt;
//     }
//
//     ret = avformat_find_stream_info(fmt_ctx, nullptr);
//     if (ret < 0) {
//         qDebug() << "找 stream info 失败！";
//         return nullopt;
//     }
//     //查找视频流
//     for (int i = 0; i < fmt_ctx->nb_streams; i++) {
//         if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//             video_stream_idx = i;
//             break;
//         }
//     }
//     if (video_stream_idx == -1)
//     {
//         qDebug() << "video_stream_idx = -1";
//         return nullopt;
//     }
//     if (!fmt_ctx)
//     {
//         qDebug() << "fmt_ctx有问题";
//     }
//
//     if (!fmt_ctx->streams[video_stream_idx]) {
//         qDebug() << "fmt_ctx->streams[" << video_stream_idx << "] 是 nullptr";
//         return nullopt;
//     }
//     AVCodecParameters* video_codec_par = fmt_ctx->streams[video_stream_idx]->codecpar;
//     const AVCodec* decoder = avcodec_find_decoder(video_codec_par->codec_id);
//
//     video_codec_ctx = avcodec_alloc_context3(decoder);
//     avcodec_parameters_to_context(video_codec_ctx,video_codec_par);
//     avcodec_open2(video_codec_ctx,decoder,nullptr);
//
//
//     for (int i = 0; i < fmt_ctx->nb_streams; i++) {
//         if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//             audio_stream_idx = i;
//             break;
//         }
//     }
//
//     if (audio_stream_idx == -1) { qDebug() << "audio_stream_idx == -1"; return nullopt; }
//     AVCodecParameters* audio_par = fmt_ctx->streams[audio_stream_idx]->codecpar;
//     const AVCodec* audio_decoder = avcodec_find_decoder(audio_par->codec_id);
//
//     if (!audio_decoder) {qDebug() << "找不到对应的音频解码器"; return nullopt; }
//
//     audio_codec_ctx = avcodec_alloc_context3(audio_decoder);
//
//     if (!audio_codec_ctx) { qDebug() << "无法分配音频 codec context"; return nullopt; }
//
//     if (avcodec_parameters_to_context(audio_codec_ctx, audio_par) < 0) { qDebug() << "无法将 codec 参数拷贝到 context"; return nullopt; }
//
//     // 5. 打开音频解码器
//     if (avcodec_open2(audio_codec_ctx, audio_decoder, nullptr) < 0) { qDebug() << "打开音频解码器失败"; return nullopt; }
//
//     // 6. 分配音频帧
//
//     AVStream* videoStream = fmt_ctx->streams[video_stream_idx];
//     //audio_frame = av_frame_alloc();
//     fmt.audio_format.sample_rate = audio_codec_ctx->sample_rate;
//     fmt.audio_format.channels = audio_codec_ctx->channels;
//     fmt.audio_format.sample_fmt = audio_codec_ctx->sample_fmt;
//     fmt.width = video_codec_par->width;
//     fmt.height = video_codec_par->height;
//     fmt.start_pts = videoStream->start_time * av_q2d(videoStream->time_base);
//     fmt.fps = av_q2d(videoStream->avg_frame_rate);
//     //fmt.duration = fmt_ctx->duration / 1000000.0;
//     fmt.duration = videoStream->duration * av_q2d(videoStream->time_base);
//     fmt.duration_ms = fmt_ctx->duration;
//     emit format_ready(fmt);
//     //return fmt;
// }

// // 3. 视频帧处理
//             if (pkt->stream_index == video_stream_idx)
//             {
//                 if (avcodec_send_packet(codecCtx, pkt) == 0)
//                 {
//                     while (avcodec_receive_frame(codecCtx, video_frame) == 0)
//                     {
//                         AVStream* stream = fmt_ctx->streams[video_stream_idx];
//                         double timestamp;
//                         if (video_frame->pts != AV_NOPTS_VALUE)
//                         {
//                             timestamp = video_frame->pts * av_q2d(stream->time_base);
//                         } else if (pkt->pts != AV_NOPTS_VALUE)
//                         {
//                             timestamp = pkt->pts * av_q2d(stream->time_base);
//                         } else
//                         {
//                             timestamp = -1; // fallback
//                         }
//                         if (check_video_format())
//                         {
//                             emit finished();
//                             return;
//                         }
//                         sws_scale(swsCtx,
//                                   video_frame->data, video_frame->linesize,
//                                   0, codecCtx->height,
//                                   tar_frame->data, tar_frame->linesize);
//
//                         render_thread->handle_add_video_frame(move(video_data->data),codecCtx->width,codecCtx->height,tar_frame->linesize[0],timestamp);
//                     }
//                 }
//             }
// // 4. 音频帧处理
//             else if (audio_stream_idx != -1 && pkt->stream_index == audio_stream_idx)
//             {
//                 if (avcodec_send_packet(audio_codec_ctx, pkt) == 0)
//                 {
//                     if (!audio_frame)
//                     {
//                         qWarning() << "Failed to allocate audio frame";
//                         emit finished();
//                         return;
//                     }
//                     if (!check_audio_format(obtained_spec))
//                     {
//                         emit finished();
//                     }
//
//                     while (avcodec_receive_frame(audio_codec_ctx, audio_frame) == 0)
//                     {
//                         AVStream* stream = fmt_ctx->streams[audio_stream_idx];
//                         double timestamp;
//
//                         if (audio_frame->pts != AV_NOPTS_VALUE) {
//                             timestamp = audio_frame->pts * av_q2d(stream->time_base);
//                         } else if (pkt->pts != AV_NOPTS_VALUE) {
//                             timestamp = pkt->pts * av_q2d(stream->time_base);
//                         } else {
//                             timestamp = -1;
//                         }
//
//                         int out_samples = av_rescale_rnd(
//                             swr_get_delay(swr_ctx, audio_codec_ctx->sample_rate) + audio_frame->nb_samples,
//                             obtained_spec.freq,
//                             audio_codec_ctx->sample_rate,
//                             AV_ROUND_UP
//                         );
//
//                         // 创建新的 AudioData 实例
//                         uint8_t** out_buf = nullptr;
//                         int ret = av_samples_alloc_array_and_samples(
//                             &out_buf,
//                             nullptr,
//                             obtained_spec.channels,
//                             out_samples,
//                             (AVSampleFormat)cur_decode_audio_type.load(),
//                             0
//                         );
//
//                         if (ret < 0 || !out_buf || !out_buf[0]) {
//                             qWarning() << "Failed to allocate audio buffer";
//                             continue;
//                         }
//
//                         int converted = swr_convert(
//                             swr_ctx,
//                             out_buf,
//                             out_samples,
//                             (const uint8_t**)audio_frame->data,
//                             audio_frame->nb_samples
//                         );
//
//                         if (converted <= 0) {
//                             qWarning() << "Audio swr_convert failed";
//                             av_freep(&out_buf[0]);
//                             av_freep(&out_buf);
//                             continue;
//                         }
//
//                         // buffer size in bytes
//                         int buffer_size = av_samples_get_buffer_size(
//                             nullptr,
//                             obtained_spec.channels,
//                             converted,
//                             (AVSampleFormat)cur_decode_audio_type.load(),
//                             1
//                         );
//
//                         audio_thread->handle_add_audio_frame(std::make_unique<AudioData>(out_buf, buffer_size, timestamp));
//                     }
//                     av_frame_free(&audio_frame);
//                 }
//             }
//             av_packet_unref(pkt);
//         }