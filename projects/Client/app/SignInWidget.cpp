//
// Created by ix on 25-5-16.
//
#pragma once
// You may need to build the project (run Qt uic code generator) to get "ui_SignInWidget.h" resolved

#include "SignInWidget.h"
#include "ui_SignInWidget.h"

void SignInWidget::set_connect()
{
    connect(ui->btn_back,&QPushButton::clicked,this,&SignInWidget::on_btn_back_clicked);
}

SignInWidget::SignInWidget(QWidget *parent, std::shared_ptr<ix::m_boost::Client::Client> client) :
    QWidget(parent), client(client), ui(new Ui::SignInWidget) {
    ui->setupUi(this);
    set_connect();
}

SignInWidget::~SignInWidget() {
    delete ui;
}

void SignInWidget::on_btn_back_clicked()
{
    this->close();
}