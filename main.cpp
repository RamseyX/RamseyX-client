/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>, et al.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/
#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>
#include <QSharedMemory>
#include <QSettings>
#include <QMessageBox>
extern "C"
{
#include "dhry.h"
}
#if defined(_WIN32)
#include <wtypes.h>
#include <processthreadsapi.h>
#elif defined(__unix__) || defined(__linux__)
#include <sys/time.h>
#include <sys/resource.h>
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
    bool isAuto = a.arguments().contains("--auto");

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
    if (!w.isInitialized())
        return 1;

    // Show main window
    if (!isAuto)
    {
        QThread::sleep(2);
        w.show();
        splash.finish(&w);
    }

    // Set process priority
#if defined(_WIN32)
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#elif defined(__unix__) || defined(__linux__)
    setpriority(PRIO_PROCESS, 0, 20);
#else
    #error Unsupported OS
#endif

    int ret = a.exec();

    if (w.shouldUpdateRamseyXClient())
    {
#if defined(_WIN32)
        ShellExecute(nullptr, TEXT("runas"), TEXT(".\\update_script.bat"), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__unix__) || defined(__linux__)
        execlp("sh", "sh", "./update_script.sh", nullptr);
#else
    #error Unsupported OS
#endif
    }

    return ret;
}
