//
// Created by ix on 25-5-19.
//
#pragma once
#include <QApplication>
#include <QDebug>
class MyApp : public QApplication{
    Q_OBJECT
public:
    MyApp(int argc, char *argv[]);
    bool notify(QObject *receiver, QEvent *event) override ;
};
