#include <QApplication>
#include <QPushButton>
#include "app/MainWindow.h"
#include "app/MyApp.h"

int main(int argc, char *argv[])
{
    // if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    //     QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling,true);
    //MyApp a(argc, argv);
    qputenv("QT_QPA_PLATFORM","xcb");
    QApplication app(argc,argv);
    auto window = new MainWindow();
    window->show();
    return app.exec();
}
