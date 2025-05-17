#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QTableWidget>
#include <QDebug>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QTime>
#include <QTimer>
using namespace std;

void MainWindow::initialize_all_object()
{
    ui->userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table_chatRoom->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    init_table_chatRoom();
    init_textEdit_chatRoom();
}

void MainWindow::set_connect()
{
    //connect(ui->table_chatRoom,&QTableWidget::cellDoubleClicked,this,&MainWindow::on_table_chatRoom_cellDoubleClicked);

    connect(this,&MainWindow::user_connected,this,[this](QString username)
    {

        this->onRefreshUserTable();
        if (!username.isEmpty())
        {
            this->addNotification(username + "已上线");
        }

    });
    //connect(this,&MainWindow::receive_chatRoom_message,this,&MainWindow::);
    //connect(ui->table_chatRoom,&QTableWidget::cellDoubleClicked,this,&MainWindow::on_table_chatRoom_cellDoubleClicked);
}

void MainWindow::set_callback()
{
    server->set_user_log_callback([this](string& username)
    {
        QString str = QString::fromStdString(username);
        emit this->user_connected(str);
    });
    server->set_handle_chatRoom_receive_message_callback([this]()
    {
        //qDebug() << "receive_charRoom_message";
        emit this->receive_chatRoom_message();
    });
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // MainWindow 构造函数里初始化
    notificationArea = new QWidget(this);
    notificationLayout = new QVBoxLayout(notificationArea);
    notificationLayout->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    notificationLayout->setSpacing(5);
    notificationArea->setLayout(notificationLayout);

    // 设置位置和大小（比如左下角）
    notificationArea->setGeometry(10, height() - 200, 300, 180);
    notificationArea->setAttribute(Qt::WA_TransparentForMouseEvents);  // 点击穿透
    notificationArea->raise();

    server = make_shared<ix::m_boost::Server::Server>();

    set_connect();
    set_callback();
    initialize_all_object();
}
MainWindow::~MainWindow() {
    delete ui;
}
//  运行服务器
void MainWindow::run()
{
    //ix::utility::Logger::Instance().Open("Service.log");
    try
    {
        string ip = "127.0.0.1";
        int port = 8081;

        server->register_server(ip,port);
        server->start_service();

        // QThread* thread = QThread::create([this]()
        // {
        //     this->server->run();
        //     QMetaObject::invokeMethod(this, "onServerShutdown", Qt::QueuedConnection);
        // });
        //thread->start();
    }catch (const exception& e)
    {
        cout << "service出错" << e.what() << endl;
    }
}




void MainWindow::onInsertRow(QStringList& data)
{
    int row = ui->userTable->rowCount();
    ui->userTable->insertRow(row);
    qDebug() << row;
    for (int col = 0;col < data.size();col++)
    {
        qDebug() << data[col];
        auto* item = new QTableWidgetItem(data[col]);
        ui->userTable->setItem(row,col,item);
    }
    qDebug() << "插入成功";
}

void MainWindow::onRefreshUserTable()
{
    ui->userTable->setRowCount(0);
    ui->userTable->clearContents();
    auto result = server->Query("SELECT * from users where status = 'ONLINE'");
    int i = 0;
    while (result->next())
    {
        QString username = QString::fromStdString(result->getString("username"));
        QString password = QString::fromStdString(result->getString("password"));
        QStringList sl;
        sl.append(username);
        sl.append(password);
        onInsertRow(sl);
        i++;
    }
    qDebug() << i;
}



void MainWindow::onRemoveChatRoom(int row)
{
    QMessageBox msg;
    msg.setText("确认要删除吗，此操作无法撤销");
    msg.setWindowTitle("提示");
    msg.setIcon(QMessageBox::Question);
    msg.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    //QApplication::beep();
    if (msg.exec() == QMessageBox::Yes)
    {
        this->ui->table_chatRoom->removeRow(row);
    }
}

void MainWindow::init_table_chatRoom()
{
    ui->table_chatRoom->setEditTriggers(QAbstractItemView::NoEditTriggers);
    auto res = server->Query("select * from chatRoomList");
    while (res->next())
    {
        int row = ui->table_chatRoom->rowCount();
        ui->table_chatRoom->insertRow(row);
        ui->table_chatRoom->setItem(row,0,new QTableWidgetItem(QString::fromStdString(res->getString("roomName"))));
        ui->table_chatRoom->setItem(row,1,new QTableWidgetItem(QString::fromStdString(res->getString("headCount"))));
        ui->table_chatRoom->setItem(row,2,new QTableWidgetItem(QString::fromStdString(res->getString("status"))));


        QPushButton* btn = new QPushButton("移除");
        connect(btn,&QPushButton::clicked,this,[this,row]()
        {
            this->onRemoveChatRoom(row);
        });
        ui->table_chatRoom->setCellWidget(row,3,btn);
    }
}

void MainWindow::init_textEdit_chatRoom()
{
    ui->textEdit_chatWindow->setReadOnly(true);
}

void MainWindow::addNotification(const QString& note)
{
    QString text = QTime::currentTime().toString() + " " + note;
    QLabel* label = new QLabel(text,this);
    label->setStyleSheet("QLabel { background-color: rgba(0,0,0,180); color: white; padding: 4px; border-radius: 5px; }");
    label->setWordWrap(true);

    notificationLayout->addWidget(label);
    QTimer::singleShot(5000,this,[this,label]{
        QPropertyAnimation* animation = new QPropertyAnimation(label, "windowOpacity");
        animation->setDuration(1000);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        connect(animation, &QPropertyAnimation::finished, [this, label, animation]() {
            notificationLayout->removeWidget(label);
            label->deleteLater();
            animation->deleteLater();
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    onServerShutdown();
    QMainWindow::closeEvent(e);
}

void MainWindow::on_btn_load_sql_users_clicked()
{
    onRefreshUserTable();
}

void MainWindow::onServerShutdown()
{
    server->shutdown();
}

void MainWindow::on_table_chatRoom_cellDoubleClicked(int row, int column) const
{
    cout << row << " "<< column << endl;
    if (column == 0)
    {
        ui->groupBox_chatWindow->setTitle(ui->table_chatRoom->itemAt(row,column)->text());
        qDebug() << "query_receive";
        auto res = server->Query("select * from chatMessage where roomName = '" + ui->groupBox_chatWindow->title().toStdString() + "'");
        QString text;
        while (res->next())
        {
            text.append(QString::fromStdString("\n[" + res->getString("timestamp") + "]\n"));
            text.append(QString::fromStdString(res->getString("sender") + " 说："));
            text.append(QString::fromStdString(res->getString("message") + "\n"));
        }
        ui->textEdit_chatWindow->setText(text);
    }
}

void MainWindow::on_receive_chatRoom_message() const
{

}

std::unique_ptr<sql::ResultSet> MainWindow::Query(string q)
{
    return server->Query(q);
}
