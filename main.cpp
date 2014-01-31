#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>
#include <cstring>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    MainWindow w;

    // Parse parameters
    bool isAuto = false;
    for (int i = 1; i < argc; ++i)
        if (!std::strcmp(argv[i], "-auto"))
        {
            isAuto = true;
            w.setAuto(isAuto);
            break;
        }

    // Show splash screen
    QSplashScreen splash(QPixmap(":/bmp/splash"));
    if (!isAuto)
    {
        splash.show();
        splash.showMessage("Initializing...",
                           Qt::AlignRight | Qt::AlignBottom,
                           Qt::white);
    }
    QThread::sleep(2);

    // Initialize
    RXTASKINFO info;
    info.id = info.layer = 1;
    w.controller.addToCompleted(info);
    info.id = info.layer = 3;
    w.controller.addToCompleted(info);
    w.initialize();

    // Show main window
    w.show();
    if (!isAuto)
        splash.finish(&w);

    return a.exec();
}
