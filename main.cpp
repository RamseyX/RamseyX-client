#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>
#include <QSharedMemory>
#include <QSettings>
#include <QMessageBox>
#include <cstring>
extern "C"
{
#include "dhry.h"
}
#ifdef _WIN32
#include <wtypes.h>
#include <processthreadsapi.h>
#else
    #error Unsupported OS
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    // Prevent multiple instances
    QSharedMemory sharedMemory("RamseyX Client");
    if (!sharedMemory.create(1))
    {
        QMessageBox::information(
                    nullptr,
                    QObject::tr("RamseyX Client"),
                    QObject::tr("Application is already running.") + "\t\t\n",
                    QMessageBox::Ok);
        return 0;
    }

    a.setOrganizationName("RamseyX");
    a.setOrganizationDomain("www.ramseyx.org");
    a.setApplicationName("RamseyX Client");
    a.setApplicationVersion(APP_VERSION);

    // Parse parameters
    bool isAuto = false;
    for (int i = 1; i < argc; ++i)
        if (!std::strcmp(argv[i], "--auto"))
        {
            isAuto = true;
            break;
        }

    // Load benchmark record
    QSettings settings;
    double DMIPS = settings.value("benchmark").toDouble();

    // Show splash screen
    QSplashScreen splash(QPixmap(":/bmp/splash"));
    if (!isAuto)
    {
        splash.show();
        if (DMIPS <= 1.0) // Assuming all modern computers' benchmark result greater than 1.0 DMIPS
            splash.showMessage(QObject::tr("Initializing...") + "\n" +
                               QObject::tr("No benchmark record exists. Benchmarking..."),
                               Qt::AlignRight | Qt::AlignBottom,
                               Qt::white);
        else
            splash.showMessage(QObject::tr("Initializing..."),
                               Qt::AlignRight | Qt::AlignBottom,
                               Qt::white);
    }

    if (DMIPS <= 1.0)
        settings.setValue("benchmark", Benchmark());

    MainWindow w(nullptr, isAuto);

    // Show main window
    if (!isAuto)
    {
        QThread::sleep(2);
        w.show();
        splash.finish(&w);
    }

    // Set process priority
#ifdef _WIN32
    ::SetPriorityClass(::GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#else
    #error Unsupported OS
#endif

    return a.exec();
}
