#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QWidget>

class MyThread : public QThread
{
    Q_OBJECT
    int delayTime;
public:
    MyThread(QWidget* parent);
    void setDelay(int val);
protected:
    void run() override;
signals:
    void ThreadTimeOut();
};

#endif // MYTHREAD_H
