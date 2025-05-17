//
// Created by ix on 25-5-15.
//
#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../src/Dispatcher/ClientDispatcher.h"

#include "../src/client/client.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    std::shared_ptr<ix::m_boost::Client::Client> client;

void initialize_all_object();

void set_connect();

void set_callback();

explicit MainWindow(QWidget *parent = nullptr);

void run();

void on_recv_chat_lists();

void on_table_chatRoom_cellDoubleClicked(int row, int column) const;

void on_tableWidget_chat_cellDoubleClicked(int row, int column) const;

void on_recv_chat_his(QString s);

void clear();

~MainWindow() override;


private:
    Ui::MainWindow *ui;

signals:
    void connect_success();
    void login_success();
    void log_out();
    void recv_chat_lists();
    void recv_chat_his(QString s);
};


#endif //MAINWINDOW_H
