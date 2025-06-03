#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include "../src/utils/ThreadPool_singleton/ThreadPool_singleton.h"
#include <QDir>
#include <ranges>
#include <QTimer>

#include "../src/utils/formula.h"
#include "../src/utils/thread_utils.h"
using namespace std;
void MainWindow::set_connect()
{
    connect(worker,&DecoderWorker::format_ready,this,&MainWindow::handle_format_ready);
    connect(worker,&DecoderWorker::finished,this,&MainWindow::handle_video_finished,Qt::QueuedConnection);
    //文件路径
    connect(ui->treeWidget_files, &QTreeWidget::itemExpanded, this, &MainWindow::handle_item_expanded);
    connect(ui->btn_refresh_video_info,&QPushButton::clicked,this,&MainWindow::on_btn_refresh_video_info_clicked);
    //connect(this,&MainWindow::update_components,this,&MainWindow::handle_update_components);

    connect(play_timer,&QTimer::timeout,[=]
    {
        handle_update_components();
        if (render_thread)
        {
            ui->label_video->setText(QString::number(render_thread->get_buff_size()));
            ui->label_render_clock->setText(QString::number(render_thread->get_render_current_time()));
        }
        ui->label_audio->setText(QString::number(get_current_time()));
        ui->label_decode_clock->setText(QString::number(worker->get_decode_current_time()));

        //ui->label_audio_clock-setText();
    });
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<VideoInfo>("VideoInfo");
    qRegisterMetaType<DecodeType>("DecodeType");
    equip_status = {
        {
            "audio", {
                false, [&]()
                {
                    if (equip_status["audio"].first)
                    {
                        SDL_CloseAudioDevice(audioDev);
                        equip_status["audio"].first = false;
                    }
                }
            }
        },
    };

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);


    init_video_widget();
    init_file_tree();
    init_comboBox_decode_type();
    avformat_network_init();
    //初始化线程池
    //ix::thread::ThreadPool::Instance(4);

    render_thread = nullptr;
    audio_thread = nullptr;
    //elapsed_timer = new QElapsedTimer();

    //ui->frame_video_window->setAttribute(Qt::WA_NativeWindow);
    //ui->frame_video_window->setUpdatesEnabled(false);
    play_timer = new QTimer();
    play_timer->setTimerType(Qt::PreciseTimer);
    pause_start_time = 0.0;
    pause_duration = 0.0;
    audio_last_pts = 0.0;
    isStoped.store(false);
    isPaused.store(false);
    set_connect();
    qApp->installEventFilter(this); // 安装事件过滤器
    ui->frame_video_window->setStyleSheet("QFrame { "
                                          "background-image: url(:/images/bg_high_quality.png); "
                                          "background-repeat: no-repeat; "
                                          "background-position: center;  "
                                          "}");
}

//重写事件函数
    void MainWindow::showEvent(QShowEvent *ev)
    {
        QMainWindow::showEvent(ev);
        //QTimer::singleShot(11,this,&MainWindow::load_default_bg);
        //qDebug() << video_window_wrapper->isVisible();

    }
    void MainWindow::resizeEvent(QResizeEvent *event)
    {
        if (render_thread)
        {
            render_thread->set_window_resize(true);
            render_thread->set_dst_rect(ui->frame_video_window->rect());
        }
        QMainWindow::resizeEvent(event);
    }
void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Left)
    {
        qDebug() << "left";
        if (worker->get_decode_status())
        {
            qDebug() << "回退";
            worker->set_seek_offset(-2.0);
        }
    }
    if (e->key() == Qt::Key_Right)
    {
        if (worker->get_decode_status())
        {
            qDebug() << "快进";
            worker->set_seek_offset(2.0);
        }
    }
    // if (e->key() == Qt::Key_Left)
    // {
    //     if (worker->get_decode_status())
    //     {
    //         worker->set_seek_offset(1.0);
    //     }
    // }
}
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (worker->get_decode_status())
        {
            if (keyEvent->key() == Qt::Key_Space)
            {
                bool isCheck = ui->btn_play->isChecked();
                on_btn_play_clicked(!isCheck);
                ui->btn_play->setChecked(!isCheck);
                qDebug() << "拦截到空格";
                return true; // 表示事件被吃掉了，不再往下传
            }
            if (keyEvent->key() == Qt::Key_Left)
            {
                worker->set_seek_offset(-1.0);
                qDebug() << "拦截到左符号";
                return true;
            }
            if (keyEvent->key() == Qt::Key_Right)
            {
                worker->set_seek_offset(1.0);
                qDebug() << "拦截到右符号";
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

//初始化
void MainWindow::init_video_widget()
{
    worker = new DecoderWorker();
    thread = new QThread(this);
    worker->moveToThread(thread);
    thread->start();

}
void MainWindow::init_file_tree()
{
    currentDir = QCoreApplication::applicationDirPath();
    QString parentPath = QFileInfo(currentDir).dir().absolutePath();

    // 创建一个根节点
    QTreeWidgetItem *root = new QTreeWidgetItem(ui->treeWidget_files);
    root->setText(0,QFileInfo(parentPath).fileName());  // 显示路径名
    root->setData(0, Qt::UserRole, parentPath); // 把路径绑定进去
    load_sub_directories(root);
    root->setExpanded(true); // 默认展开（可选）
}
void MainWindow::init_video_slider()
{
    ui->horizontalSlider_video->setMinimum(0);
    ui->horizontalSlider_video->setMaximum(100);
}
void MainWindow::init_comboBox_decode_type()
{
    for (auto value : magic_enum::enum_values<VideoType>())
    {
        if (value == VideoType::None) continue;
        ui->comboBox_decode_type->addItem(
            QString::fromStdString(std::string(magic_enum::enum_name(value))),
                QVariant::fromValue(static_cast<int>(value))
            );

        if (value == VideoType::RGB32) {
            int index = ui->comboBox_decode_type->count() - 1;
            ui->comboBox_decode_type->setCurrentIndex(index);
        }
    }
}
//自动槽函数
    void MainWindow::on_btn_play_clicked(bool value)
    {
        if (worker->get_decode_status())
        {
            isPaused.store(!value);
            pause_cv.notify_all();
            if (value)
            {
                //pause_duration += elapsed_timer->elapsed() - pause_start_time;
                play_timer->start();
                ui->btn_play->setText("stop");
            }
            else
            {
                //pause_start_time = elapsed_timer->elapsed();
                play_timer->stop();
                ui->btn_play->setText("play");
            }
        }
        else
        {
            ui->btn_play->setText("play");
            ui->btn_play->setChecked(false);
        }
    }
    void MainWindow::on_treeWidget_files_itemDoubleClicked(QTreeWidgetItem* item,int column)
    {
        QString path = item->data(0, Qt::UserRole).toString();
        QFileInfo info(path);
        QString suffix = info.suffix();
        if (suffix == "") return;
        qDebug() << path;

        if (worker->get_decode_status())
        {
            qDebug() << "在解码？先暂停等stop";
            //on_btn_play_clicked(true);
            isStoped.store(true);
            //别忘了要先解除暂停
            isPaused.store(false);
            pause_cv.notify_all();

            worker->wait_to_finished();
            isStoped.store(false);
        }
        if (suffix == "mp4")
        {
            emit worker->start_pretreatment(DecodeType::VideoWithAudio, path.toStdString());
        }
        else if (suffix == "mp3")
        {
            emit worker->start_pretreatment(DecodeType::Audio, path.toStdString());
        }
        else
        {
            emit worker->start_pretreatment(DecodeType::UNKNOWN, path.toStdString());
        }

    }
    void MainWindow::on_btn_refresh_video_info_clicked()
    {
        auto fmt = worker->get_video_info();
        QString text = QString(
            QString::number(fmt.width) + "x" + QString::number(fmt.height)
        );
        ui->textEdit_video_info->setText(text);
    }
    void MainWindow::on_comboBox_decode_type_currentIndexChanged(int index)
    {
        worker->set_decode_video_type(static_cast<AVPixelFormat>(ui->comboBox_decode_type->currentData().toInt()));
    }
    void MainWindow::on_horizontalSlider_video_sliderMoved(int value)
    {
        // double seek_val = value * 1.0 / ui->horizontalSlider_video->maximum();
        // ui->label_time_elapsed->setText(QString::fromStdString(get_time_string_from_second(value/1000000)));
        // worker->seek(seek_val);
        ui->label_time_elapsed->setText(QString::fromStdString(get_time_string_from_second(ui->horizontalSlider_video->value() / 1000000)));
        slider_busy = true;
    }
    void MainWindow::on_horizontalSlider_video_sliderReleased()
    {
        //render_thread->set_render_one_frame(false);
        if (worker->get_decode_status())
        {
            //ui->horizontalSlider_video->setValue(ui->horizontalSlider_video->value)
            //qDebug() << "%  " << video_info.duration_ms / 1000000.0 * ui->horizontalSlider_video->value() / ui->horizontalSlider_video->maximum();
            worker->set_seek_offset(video_info.duration_ms / 1000000.0 * ui->horizontalSlider_video->value() / ui->horizontalSlider_video->maximum() - get_current_time());

        }
        slider_busy = false;
    }
//自定义槽函数
void MainWindow::handle_item_expanded(QTreeWidgetItem *item)
{
    if (item->childCount() == 1 && item->child(0)->data(0, Qt::UserRole).toString() == "__DUMMY__")
    {
        item->removeChild(item->child(0));  // 删除 dummy
        load_sub_directories(item);
    }
}
void MainWindow::load_sub_directories(QTreeWidgetItem *item)
{
    QString path = item->data(0, Qt::UserRole).toString();
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& entry : entries)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem(item);
        child->setText(0, entry.fileName());
        child->setData(0, Qt::UserRole, entry.absoluteFilePath());

        if (entry.isDir())
        {
            QDir subDir(entry.absoluteFilePath());
            QFileInfoList subEntries = subDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
            if (!subEntries.isEmpty())
            {
                QTreeWidgetItem *dummy = new QTreeWidgetItem(child);
                dummy->setData(0, Qt::UserRole, "__DUMMY__");
            }
        }
    }
}
//每次播放新视频时初始化
void MainWindow::init_start_video()
{
    pause_duration = 0.0;

    //elapsed_timer->start();
    play_timer->start(5);

    isStoped.store(false);
    //ui->horizontalSlider_video->setMinimum(0);
    ui->horizontalSlider_video->setMaximum(static_cast<int>(video_info.duration_ms));
    ui->label_time_total->setText(QString::fromStdString(get_time_string_from_second(video_info.duration)));

    // 音频线程初始化（如果有音频）
    if (video_info.type != DecodeType::Video) {
        audio_thread = new AudioThread(video_info, &isPaused);
        audio_thread->start();
    }
    //只要不是音频
    if (video_info.type != DecodeType::Audio)
    {
        // 渲染线程初始化
        bool b = video_info.type == DecodeType::VideoWithAudio;
        render_thread = new RenderThread(
            (void*)ui->frame_video_window->winId(),
            b ? audio_thread->get_audio_clock() : nullptr,
            &isPaused);

        render_thread->set_dst_w_h(ui->frame_video_window->width(), ui->frame_video_window->height());
        render_thread->start();
    }
    if (video_info.type != DecodeType::Video) {
        get_current_time = [this]() {
            return audio_thread->get_audio_current_time();
        };
    } else {
        get_current_time = [this]() {
            return render_thread->get_render_current_time();
        };
    }
}

void MainWindow::handle_format_ready(VideoInfo fmt)
{
    qDebug() << "handle_audio_format_ready";
    video_info = fmt;
    init_start_video();
    if (video_info.type != DecodeType::UNKNOWN)
    {
        ui->btn_play->setText("stop");
        ui->btn_play->setChecked(true);
        emit worker->decode_ready(&isStoped, audio_thread, render_thread);
    }
}
void MainWindow::handle_update_components()
{

    //在拖动就不要刷新了
    if (!worker->get_isSeek() && !slider_busy)
    {
        double curPts = get_current_time();
        QString curTime = QString::fromStdString(get_time_string_from_second(static_cast<int>(curPts)));
        ui->label_time_elapsed->setText(curTime);
        ui->horizontalSlider_video->setValue(static_cast<int>(curPts * 1000000));
    }
}

void MainWindow::handle_video_finished()
{
    on_btn_play_clicked(false);
    ui->horizontalSlider_video->setValue(0);
    ui->label_time_total->setText("00:00:00");
    ui->label_time_elapsed->setText("00:00:00");
    play_timer->stop();

    update();

    if (render_thread)
    {
        qDebug() << "render_thread from MainWindow: ";
        delete render_thread;
        render_thread = nullptr;
    }
    if (audio_thread)
    {
        //qDebug() << "audio_thread des: " << audio_thread.get();
        delete audio_thread;
        audio_thread = nullptr;
    }
}

void MainWindow::load_default_bg()
{

}


MainWindow::~MainWindow() {
    // 等待线程退出

    isStoped.store(true);

    thread->quit();
    thread->wait();

    
    pause_cv.notify_all();
    delete worker;
    for (auto &[fst, snd]: equip_status | views::values)
    {
        if (fst)
        {
            snd();
        }
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();  // 彻底关闭 SDL

    delete ui;
}
