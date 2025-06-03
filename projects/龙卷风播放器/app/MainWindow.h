//
// Created by ix on 25-5-18.
//
#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <deque>
#include <QMainWindow>
#include <QKeyEvent>
#include <QLabel>
#include <qtreewidget.h>
#include <SDL2/SDL.h>

#include "../src/ffmpeg/DecoderWorker.h"
#include "../src/Enums/Enums.h"
#include "../src/utils/MemoryPool/MemoryPool.h"
#include "../src/utils/HighQualityTimer/HighQualityTimer.h"
#include "../src/RenderThread/RenderThread.h"
#include "../src/GlobalVariable.h"
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(VideoInfo)
Q_DECLARE_METATYPE(DecodeType)
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE




class MainWindow : public QMainWindow {
Q_OBJECT
public:
    void set_connect();

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

//事件重写
void showEvent(QShowEvent *ev) override ;
void resizeEvent(QResizeEvent *event) override;
void keyPressEvent(QKeyEvent *e) override;

bool eventFilter(QObject *obj, QEvent *event);

private:
    //初始化
    void init_video_widget();
    void init_file_tree();
    void init_video_slider();

void init_comboBox_decode_type();

void load_sub_directories(QTreeWidgetItem *item);
    void init_start_video();
//释放资源
    void deinit_end_video();

void handle_format_ready(VideoInfo fmt);

private:
    Ui::MainWindow *ui;
    QString m_videoPath;
    QThread *thread;
    DecoderWorker *worker;

    SDL_AudioDeviceID audioDev;
//文件路径
    QString currentDir;
//音频初始化
    SDL_AudioSpec wanted_spec, obtained_spec;

//音视频同步
    double audio_last_pts;
    QElapsedTimer* elapsed_timer;
    QTimer* play_timer;
    qint64 pause_duration;
    qint64 pause_start_time;
    VideoInfo video_info;

//播放设置
    std::atomic<bool> isPaused, isStoped;

//渲染线程

    //std::unique_ptr<RenderThread,std::function<void(RenderThread*)>> render_thread;
    RenderThread* render_thread;
//音频线程
    AudioThread* audio_thread;
    //std::unique_ptr<AudioThread,std::function<void(AudioThread*)>> audio_thread;
    //MemeryPool<AudioFrame> audio_frame_pool;
    //std::deque<std::unique_ptr<AudioFrame,std::function<void(AudioFrame*)>>> audio_frame;
//设备开启状态
std::unordered_map<std::string,std::pair<bool,std::function<void()>>> equip_status;
    //检测是否有音频，获取的视频进度时间
    std::function<double()> get_current_time;
    //用于显示默认图片
    QLabel* video_window_wrapper;
//拖动进度条
    bool slider_busy;
private slots:
    void on_btn_play_clicked(bool value);
    void on_treeWidget_files_itemDoubleClicked(QTreeWidgetItem* item,int column);
    void on_comboBox_decode_type_currentIndexChanged(int index);
    void on_horizontalSlider_video_sliderMoved(int value);
    void on_horizontalSlider_video_sliderReleased();

//自定义槽函数
private:
    void handle_item_expanded(QTreeWidgetItem *item);

void on_btn_refresh_video_info_clicked();

    void handle_update_components();
public:
    void handle_video_finished();


void load_default_bg();

signals:
    void prepare_to_play(std::string path);
    void update_components(double pts);

};


#endif //MAINWINDOW_H
