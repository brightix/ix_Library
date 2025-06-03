//
// Created by ix on 25-6-3.
//
#ifndef PAUSEELPASEDTIMER_H
#define PAUSEELPASEDTIMER_H
#include <qglobal.h>
#include <QDebug>

class PauseElapsedTimer {
public:
    void start() {
        _elapsed.start();
        _elapsedBeforePause = 0;
        _isPaused = false;
    }

    void pause() {
        if (!_isPaused) {
            _elapsedBeforePause += _elapsed.elapsed();
            _isPaused = true;
        }
    }

    void resume() {
        if (_isPaused) {
            _elapsed.restart(); // 重新开始新的周期
            _isPaused = false;
        }
    }

    qint64 elapsed(){
        if (_isPaused) {
            return _elapsedBeforePause;
        } else {
            return _elapsedBeforePause + _elapsed.elapsed();
        }
    }

private:
    QElapsedTimer _elapsed;
    qint64 _elapsedBeforePause = 0;
    bool _isPaused = false;
};



#endif //PAUSEELPASEDTIMER_H
