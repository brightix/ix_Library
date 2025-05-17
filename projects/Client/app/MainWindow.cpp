#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <string>
#include <QDebug>
#include <QMessageBox>

#include "../src/utility/JsonInterpreter/JsonInterpreter.h"
using namespace std;
void MainWindow::initialize_all_object()
{
    ui->tableWidget_chat->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_chat->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::set_connect()
{
    connect(ui->btn_log_out,&QPushButton::clicked,[this]()
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "提示",
            "确认要登出吗？",
            QMessageBox::Yes | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
        {
            emit log_out();
        }
    });
    connect(ui->tableWidget_chat,&QTableWidget::cellDoubleClicked,this,&MainWindow::on_tableWidget_chat_cellDoubleClicked);
    connect(this,&MainWindow::recv_chat_lists,this,&MainWindow::on_recv_chat_lists);
    connect(this,&MainWindow::recv_chat_his,this,&MainWindow::on_recv_chat_his);
}
void MainWindow::set_callback()
{
    //client->set_on_end_ping_callback();
    client->set_on_recv_chat_list_callback([this]()
    {
        emit recv_chat_lists();
    });
    client->set_on_recv_chat_his_callback([this](string& s)
    {
        emit recv_chat_his(QString::fromStdString(s));
    });
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    client = make_shared<ix::m_boost::Client::Client>();
    initialize_all_object();
    set_connect();
    set_callback();
}

void MainWindow::run()
{
    client->start_service();
}

void MainWindow::on_recv_chat_lists()
{
    //ui->tableWidget_chat->clear();
    ui->tableWidget_chat->setRowCount(0);
    auto list = client->get_chat_lists();
    for (auto l : list)
    {
        int row = ui->tableWidget_chat->rowCount();
        ui->tableWidget_chat->insertRow(row);
        for (int i = 0; i<l.size(); i++)
        {
            ui->tableWidget_chat->setItem(row,i,new QTableWidgetItem(QString::fromStdString(l[i])));
        }
    }
}

void MainWindow::on_table_chatRoom_cellDoubleClicked(int row, int column) const
{
    if (column == 0)
    {
        QString roomName = ui->tableWidget_chat->itemAt(row,column)->text();
        ui->groupBox_chat_window->setTitle(roomName);
        auto data = ix::utility::JsonInterpreter::packaging_recv_chat_his(roomName.toStdString());
        client->send_to(data->first,data->second);
    }
}
void MainWindow::on_tableWidget_chat_cellDoubleClicked(int row, int column) const
{
    qDebug() << "double clicked";
    string request_name = ui->tableWidget_chat->item(row,1)->text().toStdString();
    qDebug() << QString::fromStdString(request_name);
    client->request_chat_his(request_name);
    ui->groupBox_chat_window->setTitle(QString::fromStdString(request_name));
}

void MainWindow::on_recv_chat_his(QString s)
{
    qDebug() << "recv_his";
    ui->textEdit_chat_window->setText(s);
}

void MainWindow::clear()
{
    ui->groupBox_chat_window->setTitle("欢迎使用ix聊天平台");
    ui->tableWidget_chat->setRowCount(0);
    ui->textEdit_chat_window->setText("");
}

MainWindow::~MainWindow() {
    delete ui;
}


