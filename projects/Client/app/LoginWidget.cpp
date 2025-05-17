#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include <string>
#include "../src/utility/JsonInterpreter/JsonInterpreter.h"
#include <QMessageBox>
#include <QDebug>
#include "SignInWidget.h"
using namespace std;
void LoginWidget::set_connect()
{
    connect(ui->btn_login,&QPushButton::clicked,this,&LoginWidget::on_btn_login_clicked);
    connect(ui->btn_connect,&QPushButton::clicked,this,&LoginWidget::on_btn_connect_clicked);
    connect(this,&LoginWidget::can_check_login,this,&LoginWidget::on_login);
    connect(this,&LoginWidget::can_check_connect,this,&LoginWidget::on_connect);
    connect(this,&LoginWidget::can_check_ping,this,&LoginWidget::on_ping);
    connect(ui->btn_sign_in,&QPushButton::clicked,this,&LoginWidget::on_btn_sign_in_clicked);
}

void LoginWidget::set_callback()
{
    father->client->set_on_connect([this]()
    {
        emit can_check_connect();
    });
    father->client->set_on_login([this]()
    {
        emit can_check_login();
    });
    father->client->set_on_end_ping_callback([this](int p)
    {
        emit can_check_ping(p);
    });
}

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(nullptr), ui(new Ui::LoginWidget) ,father(dynamic_cast<MainWindow *>(parent)) {
    ui->setupUi(this);
    set_connect();
    set_callback();
    sign = new SignInWidget(nullptr,father->client);
    sign->close();
}

void LoginWidget::on_btn_login_clicked()
{
    if (!father->client->has_connect())
    {
        QMessageBox::warning(this,"警告","你还没有连接至服务器");
        return;
    }
    string username = ui->lineEdit_username->text().toStdString();
    string password = ui->lineEdit_password->text().toStdString();
    optional<pair<size_t,shared_ptr<vector<char>>>> totalMsg = ix::utility::JsonInterpreter::packaging_login(username,password);
    if (totalMsg)
    {
        father->client->send_to(totalMsg->first,totalMsg->second);
    }
}

void LoginWidget::on_btn_sign_in_clicked()
{
    setWindowTitle("注册");
    sign->show();
    //father->client->sign_in();
}

void LoginWidget::on_btn_connect_clicked() const
{
    string ip = ui->lineEdit_ip->text().toStdString();
    int port = ui->lineEdit_port->text().toInt();
    if (!father->client->has_connect())
    {
        father->client->connect(ip,port);
    }

}

void LoginWidget::on_connect()
{
    if (father->client->has_connect())
    {
        QMessageBox::information(this, "提示", "网络连接成功！");
    }
    else
    {
        QMessageBox::information(this, "提示", "网络连接失败！");
        if (!this->isVisible())
        {
            QMessageBox::information(this, "提示", "请重新连接！");
            emit disconnect();
        }
    }
}

void LoginWidget::on_login()
{
    if (father->client->has_logined())
    {
        QMessageBox::information(this, "提示", "登陆成功！");
        emit login_success(); // 通知外部打开主窗口（推荐）
    }
    else
    {
        QMessageBox::information(this, "提示", "账号或密码错误！");
    }
}

void LoginWidget::on_ping(int p)
{
    ui->label_ping_idx->setText(QString::number(p) + " ms");
}

LoginWidget::~LoginWidget() {
    delete ui;
}
