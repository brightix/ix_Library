//
// Created by ix on 25-5-9.
//
#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>

#include "../src/server/server.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:


    explicit MainWindow(QWidget *parent = nullptr);

void run();


~MainWindow() override;


private:
//  整理函数
    void set_connect();

    void set_callback();

    void addNotification(const QString &note);

//  初始化
    void init_table_chatRoom();
    void init_textEdit_chatRoom();

    void initialize_all_object();
//  非槽函数事件
    void onRemoveChatRoom(int row);
    void onInsertRow(QStringList &data);
    void onRefreshUserTable();


//  重载事件
    void closeEvent(QCloseEvent* e) override;

    std::shared_ptr<ix::m_boost::Server::Server> server;
    Ui::MainWindow *ui;
    QWidget* notificationArea;
    QVBoxLayout* notificationLayout;
private slots:
    void on_btn_load_sql_users_clicked();
    void onServerShutdown();
    void on_table_chatRoom_cellDoubleClicked(int row, int column) const;

    void on_receive_chatRoom_message() const;

    std::unique_ptr<sql::ResultSet> Query(std::string q);

signals:
    void user_connected(QString username);
    void receive_chatRoom_message();
};


#endif //MAINWINDOW_H
