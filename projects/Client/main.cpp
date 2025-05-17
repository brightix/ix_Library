#include <QApplication>
#include <QPushButton>

#include "app/LoginWidget.h"
#include "app/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow* window = new MainWindow;
    LoginWidget* login = new LoginWidget(window);
    window->run();
    login->show();
    QObject::connect(login,&LoginWidget::login_success,[&]()
    {
        login->close();
        window->show();
        //return QApplication::exec();
    });
    QObject::connect(login,&LoginWidget::disconnect,[&]()
    {
        window->client->log_out();
        window->clear();
        window->close();
        login->show();
    });
    QObject::connect(window,&MainWindow::log_out,[&]()
    {
        window->client->log_out();
        window->clear();
        window->close();
        login->show();
    });

    return a.exec();
}
