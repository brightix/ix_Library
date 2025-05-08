#include "mythread.h"

MyThread::MyThread(QWidget *parent) :QThread(parent)
{

}

void MyThread::setDelay(int val)
{
    delayTime = val;
}

void MyThread::run()
{
    while(true)
    {
        msleep(delayTime);
        emit ThreadTimeOut();
    }
}
