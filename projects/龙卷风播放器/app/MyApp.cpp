#include "MyApp.h"
//
// Created by ix on 25-5-19.
//

MyApp::MyApp(int argc, char *argv[]) : QApplication(argc,argv){}

bool MyApp::notify(QObject *receiver, QEvent *event)
{
    try {
        return QApplication::notify(receiver, event);
    } catch (const std::exception &e) {
        qDebug() << "Exception caught:" << e.what();
        return true; // 不要吞掉异常，继续抛
    } catch (...) {
        qDebug() << "Unknown exception caught.";
        return true; // 继续抛
    }
}
