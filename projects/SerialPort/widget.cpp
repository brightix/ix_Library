#include "widget.h"
#include "ui_widget.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <string>
#include "utils.h"
#include "mycombobox.h"
void Widget::init(){
    //预分配按钮提高效率
    int textsSize = 10;
    textsButtons.resize(textsSize);
    textsCheckBoxes.resize(textsSize);
    textsLineEdit.resize(textsSize);
    for(int i = 0;i<textsSize;i++)
    {
        auto btn = findChild<QPushButton*>(QString("pushButton_%1").arg(i+1));
        if(btn)
        {
            connect(btn,&QPushButton::clicked,this,[=](){
                on_pushButton_X_clicked(i);
            });
        }
        textsCheckBoxes[i] = findChild<QCheckBox*>(QString("checkBox_%1").arg(i+1));
        textsLineEdit[i] = findChild<QLineEdit*>(QString("lineEdit_%1").arg(i+1));
    }



    ui->label_sendStatus->setText(ui->comboBox_serial_port->currentText() + " is not open!");


    ui->label_time->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    ui->btn_send->setEnabled(false);
    ui->checkBox_timedTransmission->setEnabled(false);
    ui->checkBox_sendNewLine->setEnabled(false);
    ui->checkBox_HEXSend->setEnabled(false);
    ui->checkBox_HEXShow->setEnabled(false);
}

void Widget::setConnect(){
    connect(serialPort,&QSerialPort::readyRead,this,&Widget::on_SerialData_ready_to_read);

    //定时器按照用户输入的间隔时间触发发送任务
    connect(timer,&QTimer::timeout,[=](){
        on_btn_send_clicked();
    });

    //如果串口连接成功，就开启定时发送功能
    connect(ui->btn_serialSwitch,&QPushButton::toggled,this,[=](bool checked){
        ui->checkBox_timedTransmission->setEnabled(checked);
        ui->checkBox_sendNewLine->setEnabled(checked);
        ui->checkBox_HEXSend->setEnabled(checked);
        ui->checkBox_HEXShow->setEnabled(checked);
        ui->checkBox_autoEndl->setEnabled(checked);
        ui->checkBox_recvTime->setEnabled(checked);
    });
    connect(ui->btn_serialSwitch,&QPushButton::toggled,this,[=](bool checked){
        ui->checkBox_timedTransmission->setEnabled(checked);
    });
    //如果定时发送被按下，就禁用发送文字框和发送间隔
    connect(ui->checkBox_timedTransmission,&QPushButton::toggled,this,[=](bool checked){
        ui->lineEdit_timedTransmission->setEnabled(!checked);
        ui->lineEdit_send->setEnabled(!checked);
    });

    connect(currentTimer,&QTimer::timeout,this,[=]()
    {
        ui->label_time->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
    });
    //  每次切换serial_port都显示当前状态
    connect(ui->comboBox_serial_port,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){
        ui->label_sendStatus->setText(ui->comboBox_serial_port->itemText(index) + " is NOT open!");
    });
    //serial_port的刷新事件
    connect(ui->comboBox_serial_port,&MyComboBox::refresh,this,&Widget::refreshSerialName);

    connect(myThread,&MyThread::ThreadTimeOut,this,&Widget::buttons_handler);
    connect(ui->spinBox_delay_ms,QOverload<int>::of(&QSpinBox::valueChanged),this,[=](int value){
        myThread->setDelay(value);
    });
}



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setLayout(ui->gridLayout_3);
    QList<QSerialPortInfo> serialList = QSerialPortInfo::availablePorts();
    refreshSerialName();
    serialPort = new QSerialPort;

    isSerialActive = false;

    writeCntTotal = 0;
    readCntTotal = 0;
    buttonIndex = 0;
    splitSymbol = ",";

    timer = new QTimer;
    currentTimer = new QTimer;
    myThread = new MyThread(this);
    currentTimer->start(1000);
    setConnect();
    init();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btn_send_clicked()      //  发送消息
{
    QByteArray sendData = ui->lineEdit_send->text().toUtf8();
    int writeCnt{};
    if(ui->checkBox_HEXSend->isChecked()) //hex发送
    {

        QString tmp = ui->lineEdit_send->text();
        QByteArray tmpArray = tmp.toLocal8Bit();
        if(tmpArray.size() % 2)
        {
            ui->label_sendStatus->setText("Error Input");
            return;
        }
        for(char c : tmpArray)
        {
            if(!std::isxdigit(c))
            {
                ui->label_sendStatus->setText("Error Input");
                return;
            }
        }
        QByteArray arraySend = QByteArray::fromHex(tmpArray);
        writeCnt = serialPort->write(sendData);
    }
    else
    {
        writeCnt = serialPort->write(sendData);
    }
    if(writeCnt == -1)
    {
        qDebug() << "send failed!";
        ui->label_sendStatus->setText("Send Failed!");
        return;
    }


    writeCntTotal += writeCnt;
    ui->label_sentCnt->setText("send: " + QString::number(writeCntTotal));

    ui->label_sendStatus->setText("Send Ok!");


    if(QString tmp = QString::fromUtf8(sendData); tmp != sendBack)
    {
        qDebug() << tmp;
        ui->textEditRev_his->append(tmp);
        sendBack = tmp;
    }
}

void Widget::on_SerialData_ready_to_read()      //  接收消息
{
    QString recvMsg = serialPort->readAll();
    if(recvMsg.isEmpty()){
        return;
    }

    readCntTotal += recvMsg.toUtf8().size();
    if(ui->checkBox_recvTime->isChecked())
    {
        recvMsg = QDateTime::currentDateTime().toString("[dd hh:mm:ss]") + recvMsg;
    }

    QByteArray bytes = recvMsg.toUtf8();
    if(ui->checkBox_HEXShow->checkState() == Qt::Checked)//hex显示模式就转换
    {
        bytes = toHex(bytes);
    }
    if(ui->checkBox_autoEndl->isChecked())
    {
        bytes = bytes + "\r\n";
    }
    if(ui->checkBox_sendNewLine->isChecked())
    {
        bytes = "\r\n" + bytes;
    }

    ui->textEditRev->insertPlainText(bytes);
    ui->label_recvCnt->setText("recv: " + QString::number(readCntTotal));

    ui->textEditRev->moveCursor(QTextCursor::End);
    ui->textEditRev->ensureCursorVisible();
    //ui->textEditRev->setFocus();
    //qDebug() << "recv: "<< recvMsg;
}


void Widget::on_btn_serialSwitch_clicked(bool checked)
{
    if(checked)
    {
        //选择端口号
        serialPort->setPortName(ui->comboBox_serial_port->currentText());
        //配置波特率
        serialPort->setBaudRate(ui->comboBox_boautrate->currentText().toUInt());

        //配置校验位
        switch(ui->comboBox_jiaoyan->currentIndex())
        {
        case 0:
            serialPort->setParity(QSerialPort::NoParity);
            break;
        case 1:
            serialPort->setParity(QSerialPort::EvenParity);
            break;
        case 2:
            serialPort->setParity(QSerialPort::MarkParity);
            break;
        case 3:
            serialPort->setParity(QSerialPort::OddParity);
            break;
        case 4:
            serialPort->setParity(QSerialPort::SpaceParity);
            break;
        default:
            serialPort->setParity(QSerialPort::UnknownParity);
            break;
        }

        //配置数据位
        serialPort->setDataBits(QSerialPort::DataBits(ui->comboBox_databit->currentText().toUInt()));
        //配置停止位
        serialPort->setStopBits(QSerialPort::StopBits(ui->comboBox_stopbit->currentText().toInt()));
        //配置流控
        if(ui->comboBox_fileCon->currentText() == "None")
        {
            serialPort->setFlowControl(QSerialPort::NoFlowControl);
        }
        //打开窗口
        if(serialPort->open(QIODevice::ReadWrite))
        {
            qDebug() << "serial open successful";
            ui->btn_serialSwitch->setText("关闭串口");
            ui->comboBox_boautrate->setEnabled(false);
            ui->comboBox_databit->setEnabled(false);
            ui->comboBox_fileCon->setEnabled(false);
            ui->comboBox_jiaoyan->setEnabled(false);
            ui->comboBox_serial_port->setEnabled(false);
            ui->comboBox_stopbit->setEnabled(false);
            ui->btn_send->setEnabled(true);
            ui->label_sendStatus->setText(ui->comboBox_serial_port->currentText() + " is open!");
        }
        else
        {
            QMessageBox messageBox;
            messageBox.setWindowTitle("提示");
            QString erroText;
            erroText.append("串口可能被占用或未被授权\n");
            erroText.append("错误提示: " + serialPort->errorString());

            messageBox.setText(erroText);
            messageBox.exec();
            ui->btn_serialSwitch->setChecked(false);
        }
    }
    else
    {
        serialPort->close();
        ui->btn_serialSwitch->setText("打开串口");
        ui->comboBox_boautrate->setEnabled(true);
        ui->comboBox_databit->setEnabled(true);
        ui->comboBox_fileCon->setEnabled(true);
        ui->comboBox_jiaoyan->setEnabled(true);
        ui->comboBox_serial_port->setEnabled(true);
        ui->comboBox_stopbit->setEnabled(true);
        ui->btn_send->setEnabled(false);
        ui->label_sendStatus->setText(ui->comboBox_serial_port->currentText() + " is close!");

    }
}


void Widget::on_checkBox_timedTransmission_clicked(bool checked)
{
    if(checked)
    {
        timer->start(ui->lineEdit_timedTransmission->text().toInt());
    }
    else
    {
        timer->stop();
    }

}


void Widget::on_btn_clearPort_clicked()
{
    ui->textEditRev->clear();
}


void Widget::on_btn_saveRecv_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                       "/home/jana/untitled.png",
                                                       tr("Images (*.png *.xpm *.jpg)"));
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text) == -1){
        qDebug() << "打开文件失败";
        return;
    }
    QTextStream out(&file);
    out << ui->textEditRev->toPlainText();
    file.close();
}


void Widget::on_checkBox_HEXShow_stateChanged(int state)
{
    QString text = ui->textEditRev->toPlainText();
    QByteArray bytes = text.toUtf8();
    QByteArray result;
    if(state == Qt::Checked)
    {
        result = toHex(bytes);
        text = QString::fromUtf8(result).toUpper();
    }
    else
    {
        bytes = QByteArray::fromHex(bytes);
        result = bytes;
        text = QString::fromUtf8(result);
    }
    ui->textEditRev->setText(text);
}



void Widget::on_btn_hideBoard_clicked(bool checked)
{
    qDebug() << "hideBoard pushed";
    if(checked)
    {
        qDebug() << "hide";
        ui->btn_hideBoard->setText("拓展面板");
        ui->groupBoxTexts->hide();
    }
    else
    {
        qDebug() << "show";
        ui->btn_hideBoard->setText("隐藏面板");
        ui->groupBoxTexts->show();
    }
}




void Widget::on_btn_hideHis_clicked(bool checked)
{
    if(checked)
    {
        qDebug() << "hide";
        ui->btn_hideHis->setText("显示历史");
        ui->groupBoxHis->hide();
    }
    else
    {
        qDebug() << "show";
        ui->btn_hideHis->setText("隐藏历史");
        ui->groupBoxHis->show();
    }
}

void Widget::refreshSerialName()
{
    qDebug() << "refresh";
    ui->comboBox_serial_port->clear();
    QList<QSerialPortInfo> serialList = QSerialPortInfo::availablePorts();
    for(QSerialPortInfo s: serialList)
    {
        qDebug() << s.portName();
        ui->comboBox_boautrate->setCurrentIndex(6);
        ui->comboBox_databit->setCurrentIndex(3);
        ui->comboBox_serial_port->addItem(s.portName());
    }
    ui->label_sendStatus->setText("Refresh!");
}


void Widget::on_pushButton_X_clicked(int index)
{
    QString text = textsLineEdit[index]->text();
    ui->lineEdit_send->setText(text);

    if(textsCheckBoxes[index]->isChecked())
    {
        ui->checkBox_HEXSend->setChecked(true);
    }
    else
    {
        ui->checkBox_HEXSend->setChecked(false);
    }
    on_btn_send_clicked();
}

void Widget::buttons_handler()
{
    if(buttonIndex < textsButtons.size())
    {
        on_pushButton_X_clicked(buttonIndex);
        buttonIndex++;
    }
    else
    {
        buttonIndex = 0;
    }
}

void Widget::on_checkBox_periodicSend_clicked(bool checked)
{
    if(checked)
    {
        ui->spinBox_delay_ms->setEnabled(false);
        myThread->start();
    }
    else
    {
        myThread->terminate();
        ui->spinBox_delay_ms->setEnabled(true);
    }
}


void Widget::on_btn_reset_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("提示");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("重置不可逆，确认是否重置？");
    QPushButton* YesButton = msgBox.addButton("是",QMessageBox::YesRole);
    QPushButton* NoButton = msgBox.addButton("否",QMessageBox::NoRole);
    int ret = msgBox.exec();
    if(msgBox.clickedButton() == YesButton)
    {
        for(int i = 0;i<10;i++)
        {
            textsCheckBoxes[i]->setChecked(false);
            textsLineEdit[i]->clear();
        }
    }
    if(msgBox.clickedButton() == NoButton)
    {

    }
}


void Widget::on_btn_save_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Save File"),QDir::currentPath(),tr("Text (*.txt)"));
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        for(int i = 0;i<10;i++)
        {
            out << textsCheckBoxes[i]->isChecked() << splitSymbol << textsLineEdit[i]->text() << endl;
        }
        file.close();
    }
    else
    {
        qDebug() << "保存文件失败";
    }
}

//  有问题，如果出现多个分隔符，会造成数据丢失
void Widget::on_btn_load_clicked()
{
    QStringList cache;
    for(int i = 0;i<10;i++)
    {

        cache.append((textsCheckBoxes[i]->isChecked() ? "1" : "0") + splitSymbol + textsLineEdit[i]->text());
    }
    int i = 0;
    QString filename = QFileDialog::getOpenFileName(this,tr("Open File"),QDir::currentPath(),tr("Text (*.txt)"));
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        for(int i = 0;i<10;i++)
        {
            QString text = in.readLine();
            QStringList parts = text.split(",");
            if(parts.size() == 2)
            {
                textsCheckBoxes[i]->setChecked(parts[0] != "0");
                textsLineEdit[i]->setText(parts[1]);
            }
            else
            {
                qDebug() << "数据有问题，可能出现多个分隔符('"+ splitSymbol + "')";
                //  还原内容
                for(int i = 0;i<10;i++)
                {
                    QStringList p = cache[i].split(splitSymbol);
                    textsCheckBoxes[i]->setChecked(p[0] != "0");
                    textsLineEdit[i]->setText(p[1]);
                }
                return;
            }
        }
        file.close();
    }
    else
    {
        qDebug() << "保存文件失败";
    }
}

