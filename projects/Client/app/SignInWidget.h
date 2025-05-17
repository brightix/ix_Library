//
// Created by ix on 25-5-16.
//
#pragma once
#ifndef SIGNINWIDGET_H
#define SIGNINWIDGET_H
#include "../src/client/client.h"
#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class SignInWidget; }
QT_END_NAMESPACE

class SignInWidget : public QWidget {
Q_OBJECT

public:
void set_connect();

    explicit SignInWidget(QWidget *parent = nullptr,std::shared_ptr<ix::m_boost::Client::Client> client = nullptr);
    ~SignInWidget() override;

void on_btn_back_clicked();

private:
    Ui::SignInWidget *ui;
    std::shared_ptr<ix::m_boost::Client::Client> client;
};


#endif //SIGNINWIDGET_H
