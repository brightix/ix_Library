#include <iostream>
#include "app/MainWindow.h"
#include <QApplication>
#include <QPushButton>

using namespace std;
int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    MainWindow main_window;
    main_window.show();
    main_window.run();
    return QApplication::exec();
}