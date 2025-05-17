//
// Created by ix on 25-5-15.
//
#pragma once
#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "MainWindow.h"
#include "SignInWidget.h"


QT_BEGIN_NAMESPACE
namespace Ui { class LoginWidget; }
QT_END_NAMESPACE

class LoginWidget : public QWidget {
Q_OBJECT

public:

explicit LoginWidget(QWidget *parent = nullptr);

void set_connect();
void set_callback();

void on_btn_login_clicked();

void on_btn_sign_in_clicked();

void on_btn_connect_clicked() const;

void on_connect();
void on_login();
void on_ping(int p);

~LoginWidget() override;

private:
    Ui::LoginWidget *ui;
    MainWindow* father;
    SignInWidget* sign;
signals:
    void login_success();
    void disconnect();


    void can_check_connect();
    void can_check_login();
    void can_check_ping(int p);
};


#endif //LOGINWIDGET_H
