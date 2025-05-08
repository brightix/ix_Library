#ifndef WIDGET_H
#define WIDGET_H

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSerialPort>
#include <QTimer>
#include <QWidget>

#include "mythread.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private slots:
    void on_btn_send_clicked();
    void on_SerialData_ready_to_read();
    void on_btn_serialSwitch_clicked(bool checked);
    void on_checkBox_timedTransmission_clicked(bool checked);
    void on_btn_clearPort_clicked();
    void on_btn_saveRecv_clicked();
    void on_checkBox_HEXShow_stateChanged(int arg1);

    void on_btn_hideBoard_clicked(bool checked);

    void on_btn_hideHis_clicked(bool checked);
    void refreshSerialName();
    void on_pushButton_X_clicked(int index);

    void on_checkBox_periodicSend_clicked(bool checked);

    void on_btn_reset_clicked();

    void on_btn_save_clicked();

    void on_btn_load_clicked();

private:
    Ui::Widget *ui;
    QSerialPort* serialPort;
    int writeCntTotal;
    int readCntTotal;
    QString sendBack;
    bool isSerialActive;
    QTimer* timer;
    QTimer* currentTimer;

    QVector<QPushButton*> textsButtons;
    QVector<QCheckBox*> textsCheckBoxes;
    QVector<QLineEdit*> textsLineEdit;
    MyThread* myThread;
    QString splitSymbol;
    int buttonIndex;
    void init();
    void setConnect();
    void buttons_handler();
};
#endif // WIDGET_H
