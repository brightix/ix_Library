//
// Created by ix on 25-5-20.
//
#ifndef HIGHQUALITYTIMER_H
#define HIGHQUALITYTIMER_H
#include <qelapsedtimer.h>
#include <QTimer>


class MyHighPrecisionTimer : public QObject {
    Q_OBJECT
public:
    MyHighPrecisionTimer(int intervalMs, QObject* parent = nullptr)
        : QObject(parent), interval(intervalMs) 
    {
        timer.setTimerType(Qt::PreciseTimer);
        connect(&timer, &QTimer::timeout, this, &MyHighPrecisionTimer::onTimeout);
    }

    void start() {
        elapsed.restart();     // 重启计时器
        timer.start(interval);
    }

    signals:
        void timeout();

private slots:
    void onTimeout() {
    qint64 elapsedMs = elapsed.elapsed();
    if (elapsedMs >= interval) {
        emit timeout();
        elapsed.restart();
        timer.start(interval);
    } else {
        timer.start(interval - elapsedMs);  // 重新调整触发时间
    }
}

private:
    QTimer timer;
    QElapsedTimer elapsed;
    int interval;
};


#endif //HIGHQUALITYTIMER_H
