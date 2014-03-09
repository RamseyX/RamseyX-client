/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>
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
#include "ui_mainwindow.h"
#include "ramseyxcontroller.h"
#include "ramseyxutils.h"
#include <QDateTime>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QTableWidgetItem>
#include <QIcon>
#include <QCloseEvent>
#include <QSettings>
#include <QRegExp>
#include <QHostInfo>
#include <thread>
#include <exception>
#if defined(_WIN32)
#include <wtypes.h>
#include <shellapi.h>
#elif defined(__unix__) || defined(__linux__)
#else
    #error Unsupported OS
#endif

MainWindow::MainWindow(QWidget *parent, bool autoState) :
    QMainWindow(parent,
                Qt::CustomizeWindowHint |
                Qt::WindowTitleHint |
                Qt::WindowCloseButtonHint |
                Qt::WindowMinimizeButtonHint),
    ui(new Ui::MainWindow),
    isAuto(autoState)
{
    ui->setupUi(this);

    setFixedSize(size());

    isUploading.clear();
    isDownloading.clear();

    try
    {
        initialize();
        initialized = true;
    }
    catch (QString what)
    {
        QMessageBox::critical(
            this,
            tr("RamseyX Client"),
            tr("Fatal error: ") + what + "\t\t\n",
            QMessageBox::Ok);
    }
}

MainWindow::~MainWindow()
{
    // Clean up
    for (const auto &i : timersAlways)
        this->killTimer(i.second);
    for (const auto &i : timersRunning)
        this->killTimer(i.second);

    theController().pause();

    stopUiThreads();

    while (isUploading.test_and_set())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    while (isDownloading.test_and_set())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    delete ui;
}

bool MainWindow::isInitialized()
{
    return initialized;
}

bool MainWindow::shouldUpdateRamseyXClient()
{
    return updateRamseyXClient;
}

QIcon MainWindow::loadAllSizesIcons()
{
    QIcon icon;

    for (int i = 1; i <= 13; ++i)
        icon.addPixmap(QPixmap(QString(":/png/RX_%1").arg(i)));

    return icon;
}

void MainWindow::createActions()
{
    startAction = new QAction(tr("&Start!"), this);
    connect(startAction, SIGNAL(triggered()), this, SLOT(onBtnStartClicked()));

    hideAction = new QAction(tr("&Hide"), this);
    connect(hideAction, SIGNAL(triggered()), this, SLOT(hide()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));
}

void MainWindow::createTrayIcon(const QIcon &icon)
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(startAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(hideAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(tr("RamseyX Client"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(onIconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::createUiThreads()
{
    whatsUpThread = new QThread(this);
    whatsUpWorker = new WhatsUpWorker;
    whatsUpWorker->moveToThread(whatsUpThread);
    connect(whatsUpWorker, SIGNAL(send(QString)), this, SLOT(onWhatsUpThreadFinished(QString)));
    connect(whatsUpThread, SIGNAL(finished()), whatsUpWorker, SLOT(deleteLater()));
    whatsUpThread->start();

    checkForUpdateThread = new QThread(this);
    checkForUpdateWorker = new CheckForUpdateWorker;
    checkForUpdateWorker->moveToThread(checkForUpdateThread);
    connect(checkForUpdateWorker, SIGNAL(send(int, int, int, int)),
            this, SLOT(onCheckForUpdateThreadFinished(int, int, int, int)));
    connect(checkForUpdateThread, SIGNAL(finished()), checkForUpdateWorker, SLOT(deleteLater()));
    checkForUpdateThread->start();

    refreshOverallThread = new QThread(this);
    refreshOverallWorker = new RefreshOverallWorker;
    refreshOverallWorker->moveToThread(refreshOverallThread);
    connect(refreshOverallWorker, SIGNAL(send(QString, QString, QString, QString, QString, QString)),
            this, SLOT(onRefreshOverallThreadFinished(QString, QString, QString, QString, QString, QString)));
    connect(refreshOverallThread, SIGNAL(finished()), refreshOverallWorker, SLOT(deleteLater()));
    refreshOverallThread->start();

    refresh20Thread = new QThread(this);
    refresh20Worker = new Refresh20Worker;
    refresh20Worker->moveToThread(refresh20Thread);
    connect(this, SIGNAL(refresh20(int, QString, QString)), refresh20Worker, SLOT(doWork(int, QString, QString)));
    connect(refresh20Worker, SIGNAL(send(QString)), this, SLOT(onRefresh20ThreadFinished(QString)));
    connect(refresh20Thread, SIGNAL(finished()), refresh20Worker, SLOT(deleteLater()));
    refresh20Thread->start();
}

void MainWindow::stopUiThreads()
{
    if (whatsUpThread)
        whatsUpThread->quit();
    if (checkForUpdateThread)
        checkForUpdateThread->quit();
    if (refreshOverallThread)
        refreshOverallThread->quit();
    if (refresh20Thread)
        refresh20Thread->quit();

    while (whatsUpThread && !whatsUpThread->wait(1000))
        whatsUpThread->terminate();
    while (checkForUpdateThread && !checkForUpdateThread->wait(1000))
        checkForUpdateThread->terminate();
    while (refreshOverallThread && !refreshOverallThread->wait(1000))
        refreshOverallThread->terminate();
    while (refresh20Thread && !refresh20Thread->wait(1000))
        refresh20Thread->terminate();
}

void MainWindow::storeAccount()
{
    QSettings settings;
    settings.setValue("account/username", username);
    settings.setValue("account/password", password);
}

bool MainWindow::loadAccount()
{
    QSettings settings;
    QString usr(settings.value("account/username").toString());
    if (!QRegExp("[a-zA-Z0-9_]{1,16}").exactMatch(usr))
        return false;
    QString pwd(settings.value("account/password").toString());
    if (!QRegExp("[a-zA-Z0-9_]{4,16}").exactMatch(pwd))
        return false;
    username = usr;
    password = pwd;
    return true;
}

void MainWindow::storeThreadNumber()
{
    QSettings settings;
    settings.setValue("threadNumber", ui->threadSlider->value());
}

void MainWindow::loadThreadNumber()
{
    QSettings settings;
    int value = settings.value("threadNumber").toInt();
    int max = QThread::idealThreadCount();
    max = (max > RX_MAX_THREAD_NUM ? RX_MAX_THREAD_NUM : max);
    if (value >= 1 && value <= max)
    {
        theController().setMaxThreadNum(value);
        ui->threadSlider->setValue(value);
    }
    else
    {
        theController().setMaxThreadNum(max);
        ui->threadSlider->setValue(max);
        storeThreadNumber();
    }
}

void MainWindow::storeStartupSetting()
{
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (ui->runOnStartupChk->checkState() == Qt::Checked)
        settings.setValue("RamseyX Client",
                          QString("\"%1\" --auto").arg(qApp->applicationFilePath().replace('/', '\\')));
    else
        settings.remove("RamseyX Client");
#elif defined(__unix__) || defined(__linux__)
    if (ui->runOnStartupChk->checkState() == Qt::Checked)
    {
        ui->runOnStartupChk->setChecked(false);
        QMessageBox::information(
                    this,
                    tr("RamseyX Client"),
                    tr("It seems that RamseyX Client is running on a UNIX/Linux-based OS. "
                       "Due to the difference between all UNIX/Linux distributions, "
                       "we are currently unable to provide a general implementation for this function. "
                       "You might have to configure manually.") + "\t\t\n",
                    QMessageBox::Ok);
    }
#else
    #error Unsupported OS
#endif
}

void MainWindow::loadStartupSetting()
{
#ifdef _WIN32
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (settings.value("RamseyX Client").toString() == QString("\"%1\" --auto").arg(qApp->applicationFilePath().replace('/', '\\')))
        ui->runOnStartupChk->setChecked(true);
    else
        ui->runOnStartupChk->setChecked(false);
#elif defined(__unix__) || defined(__linux__)
    ui->runOnStartupChk->setChecked(false);
#else
    #error Unsupported OS
#endif
}

void MainWindow::initialize()
{
    // Set log dir
    if (!QDir().mkpath(dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation)))
        throw tr("Failed to create log directory.");
    theController().setLogDir(dir.toStdString());

    /*// Open log database
    if (!theController().openLogDatabase())
        throw tr("Failed to open log database.");*/

    // Read log
    if (!theController().readLog() && !theController().resetLog())
        throw tr("Failed to read or reset log.");

    connect(&theController(), SIGNAL(updateTasksStatus()), this, SLOT(updateTasksStatus()));
    connect(&theController(), SIGNAL(uploadFinished()), this, SLOT(onAutoUploadThreadFinished()));
    connect(&theController(), SIGNAL(downloadFinished()), this, SLOT(onAutoDownloadThreadFinished()));
    connect(&theController(), SIGNAL(allTasksCompleted()), this, SLOT(onAllTasksCompleted()));
    connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(onBtnStartClicked()));
    connect(ui->btnAccount, SIGNAL(clicked()), this, SLOT(onBtnAccountClicked()));
    connect(ui->btnWhatsUp, SIGNAL(clicked()), this, SLOT(onBtnWhatsUpClicked()));
    connect(ui->btnRefresh, SIGNAL(clicked()), this, SLOT(onBtnRefreshClicked()));
    connect(ui->resetBenchmarkAction, SIGNAL(triggered()), this, SLOT(onResetBenchmarkAction()));
    connect(ui->quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->mailingListAction, SIGNAL(triggered()), this, SLOT(onMailingListAction()));
    connect(ui->aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->aboutAction, SIGNAL(triggered()), this, SLOT(onAboutAction()));
    connect(ui->tab20, SIGNAL(currentChanged(int)), this, SLOT(onTab20IndexChanged(int)));
    connect(ui->threadSlider, SIGNAL(sliderPressed()), this, SLOT(onThreadSliderPressed()));
    connect(ui->threadSlider, SIGNAL(valueChanged(int)), this, SLOT(onThreadSliderValueChanged(int)));
    connect(ui->threadSlider, SIGNAL(sliderReleased()), this, SLOT(onThreadSliderReleased()));
    connect(ui->runOnStartupChk, SIGNAL(clicked()), this, SLOT(onRunOnStartupChkClicked()));

    // Window icon & title
    QIcon icon(loadAllSizesIcons());
    setWindowIcon(icon);
    setWindowTitle(tr("RamseyX Client ") + APP_VERSION);

    // tblOverall
    ui->tblOverall->setRowCount(1);
    ui->tblOverall->setColumnCount(6);
    ui->tblOverall->setColumnWidth(0, 75);
    ui->tblOverall->setColumnWidth(1, 55);
    ui->tblOverall->setColumnWidth(2, 85);
    ui->tblOverall->setColumnWidth(3, 75);
    ui->tblOverall->setColumnWidth(4, 155);
    ui->tblOverall->horizontalHeader()->setStretchLastSection(true);
    ui->tblOverall->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tblOverall->setHorizontalHeaderLabels(QStringList() << tr("Progress") << tr("Users")
                                              << tr("Computers") << tr("Tasks")
                                              << tr("Cumulative Time (Days)")
                                              << tr("Current / Max Power (KDIMPS)"));
    ui->tblOverall->horizontalHeaderItem(0)->setSizeHint(
                QSize(ui->tblOverall->horizontalHeaderItem(0)->sizeHint().width(), 25));
    for (int i = 0; i < 6; ++i)
    {
        ui->tblOverall->setItem(0, i, new QTableWidgetItem(QString()));
        ui->tblOverall->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignCenter);
        ui->tblOverall->item(0, i)->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    }

    // tbl20
    ui->tbl20->setRowCount(20);
    ui->tbl20->setColumnCount(6);
    ui->tbl20->setColumnWidth(0, 55);
    ui->tbl20->setColumnWidth(1, 110);
    ui->tbl20->setColumnWidth(2, 80);
    ui->tbl20->setColumnWidth(3, 115);
    ui->tbl20->setColumnWidth(4, 150);
    ui->tbl20->horizontalHeader()->setStretchLastSection(true);
    ui->tbl20->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tbl20->setHorizontalHeaderLabels(QStringList() << tr("Rank") << tr("Username")
                                              << tr("Credits") << tr("Completed Tasks")
                                              << tr("Cumulative Time (Days)")
                                              << tr("Current Power (DIMPS)"));
    ui->tbl20->horizontalHeaderItem(0)->setSizeHint(
                QSize(ui->tbl20->horizontalHeaderItem(0)->sizeHint().width(), 25));
    for (int i = 0; i < 6; ++i)
    {
        ui->tbl20->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignCenter);
        for (int j = 0; j < 20; ++j)
        {
            ui->tbl20->setItem(j, i, new QTableWidgetItem(QString()));
            ui->tbl20->item(j, i)->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
        }
    }

    // tab20
    ui->tab20->setTabEnabled(1, false);
    ui->tab20->setCurrentIndex(0);

    // trayIcon
    createActions();
    createTrayIcon(icon);
    trayIcon->show();

    // Threads
    createUiThreads();

    // Set thread slider
    int maxThreadNum = QThread::idealThreadCount();
    maxThreadNum = (maxThreadNum > RX_MAX_THREAD_NUM ? RX_MAX_THREAD_NUM : maxThreadNum);
    ui->threadSlider->setMaximum(maxThreadNum);

    // Load thread number
    loadThreadNumber();

    // Load benchmark
    benchmarkInDMIPS = QSettings().value("benchmark").toDouble();

    // Load startup setting
    loadStartupSetting();

    // Load account
    enableUserSpecificFeatures(isAccountLocked = loadAccount());

    // Set timers
    timersAlways[std::chrono::seconds(600)] = startTimer(600000); // Auto update
    timersAlways[std::chrono::seconds(30)] = startTimer(30000); // Auto upload & download
    timersAlways[std::chrono::seconds(3600)] = startTimer(3600000); // Update benchmark

    // Update tasks status
    updateTasksStatus(true);

    // Update what's up
    onBtnWhatsUpClicked();

    // Refresh
    onBtnRefreshClicked();

    // Update benchmark
    if (isAccountLocked)
        std::thread(&RamseyXController::updateBenchmark,
                    username.toStdString(), password.toStdString(),
                    QHostInfo::localHostName().toStdString(),
                    RamseyXUtils::getCpuBrandString(),
                    benchmarkInDMIPS * QThread::idealThreadCount()).detach();

    // Autorun
    if (isAuto)
        onBtnStartClicked();
}

void MainWindow::quitAndUpdate()
{
    updateRamseyXClient = true;
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

void MainWindow::onCheckForUpdateThreadFinished(int necessaryLevel, int major, int minor, int patchLevel)
{
    switch (necessaryLevel)
    {
        case RX_UNNECESSARY:
            break;
        case RX_OPTIONAL:
            if (!silentUpdate)
            {
                show();
                if (QMessageBox::Yes ==
                        QMessageBox::question(this, tr("Update Available"),
                                              tr("A new version of RamseyX Client has been released.") + "\t\t\n" +
                                              tr("Do you wish to update to %1.%2.%3 now?").arg(major).arg(minor).arg(patchLevel) +
                                              "\t\t\n", QMessageBox::Yes, QMessageBox::No))
                    quitAndUpdate();
            }
            else
                quitAndUpdate();
            break;
        case RX_NECESSARY:
            if (!silentUpdate)
            {
                show();
                QMessageBox::information(this, tr("Update Available"),
                                         tr("A new version of RamseyX Client has been released.") + "\t\t\n" +
                                         tr("Program will update to %1.%2.%3 now.").arg(major).arg(minor).arg(patchLevel) +
                                         "\t\t\n",
                                         QMessageBox::Ok);
            }
            quitAndUpdate();
            break;
        default:
            break;
    }

    isUpdating = !isUpdating;
    if (!isUpdating)
    {
        ui->btnWhatsUp->setText(tr("&Update Now"));
        ui->btnWhatsUp->setEnabled(true);
    }
}

void MainWindow::enableUserSpecificFeatures(bool enabled)
{
    if (enabled)
    {
        ui->tab20->setTabEnabled(1, true);
        ui->disableAutoUploadChk->setEnabled(true);
        ui->disableAutoDownloadChk->setEnabled(true);
        ui->disableAutoUploadChk->setChecked(false);
        ui->disableAutoDownloadChk->setChecked(false);
    }
    else
    {
        if (ui->tab20->currentIndex() == 1 && !isRefreshing)
            ui->tab20->setCurrentIndex(0);
        ui->tab20->setTabEnabled(1, false);
        ui->disableAutoUploadChk->setDisabled(true);
        ui->disableAutoDownloadChk->setDisabled(true);
        ui->disableAutoUploadChk->setChecked(true);
        ui->disableAutoDownloadChk->setChecked(true);
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    int id = event->timerId();

    if (id == timersRunning[std::chrono::seconds(1)])
        updateTasksStatus();
    else if (id == timersRunning[std::chrono::seconds(10)])
    {
        theController().clearOutdated();
        theController().writeLog();
    }
    else if (id == timersAlways[std::chrono::seconds(600)])
        onBtnWhatsUpClicked();
    else if (id == timersAlways[std::chrono::seconds(30)])
    {
        if (ui->disableAutoUploadChk->checkState() == Qt::Unchecked && !isUploading.test_and_set())
            // Auto upload
            std::thread(&RamseyXController::uploadAll, &theController(),
                        username.toStdString(), password.toStdString(),
                        QHostInfo::localHostName().toStdString(),
                        RamseyXUtils::getCpuBrandString()).detach();
        if (ui->disableAutoDownloadChk->checkState() == Qt::Unchecked && !isDownloading.test_and_set())
            // Auto download
            std::thread(&RamseyXController::fillTaskLists, &theController(),
                        username.toStdString(), password.toStdString()).detach();
    }
    else if (id == timersAlways[std::chrono::seconds(3600)])
        // Update benchmark
        if (isAccountLocked)
            std::thread(&RamseyXController::updateBenchmark,
                        username.toStdString(), password.toStdString(),
                        QHostInfo::localHostName().toStdString(),
                        RamseyXUtils::getCpuBrandString(),
                        benchmarkInDMIPS * QThread::idealThreadCount()).detach();

    QMainWindow::timerEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    trayIcon->showMessage(tr("RamseyX Client"),
                          tr("The program will keep running in the "
                            "background. To terminate the program, "
                            "choose \"Quit\" in the context menu "
                            "of the system tray entry."));
    event->ignore();
}

void MainWindow::setAccount(const QString &usr, const QString &pwd)
{
    username = usr;
    password = pwd;
}

void MainWindow::onAllTasksCompleted()
{
    if (theController().isRunning())
        onBtnStartClicked();
}

void MainWindow::onResetBenchmarkAction()
{
    QSettings().remove("benchmark");
    QMessageBox::information(
                this,
                tr("RamseyX Client"),
                tr("Benchmark record reset. Program will re-benchmark on next startup.") + "\t\t\n",
                QMessageBox::Ok);
}

void MainWindow::onMailingListAction()
{
    QMessageBox::information(
                this,
                tr("RamseyX Mailing List"),
                tr("<p>Report a bug:<br />"
                   "&nbsp;&nbsp;&nbsp;<a href=\"mailto:report-bugs@ramseyx.org\">"
                   "&lt;report-bugs@ramseyx.org&gt;</a></p>"
                   "<p>Give advice:<br />"
                   "&nbsp;&nbsp;&nbsp;<a href=\"mailto:advise@ramseyx.org\">"
                   "&lt;advise@ramseyx.org&gt;</a></p>"
                   "<p>Join our development team:<br />"
                   "&nbsp;&nbsp;&nbsp;<a href=\"mailto:join@ramseyx.org\">"
                   "&lt;join@ramseyx.org&gt;</a></p>"
                   "<p>Make friends with the author in person =ω=:<br />"
                   "&nbsp;&nbsp;&nbsp;<a href=\"mailto:zizheng.tai@gmail.com\">"
                   "&lt;zizheng.tai@gmail.com&gt;</a></p>"),
                QMessageBox::Ok);
}

void MainWindow::onAboutAction()
{
    QMessageBox::about(
                this,
                tr("About RamseyX Client"),
                tr("<h3>RamseyX Client %1.%2.%3</h3>").arg(RX_VER_MAJOR).arg(RX_VER_MINOR).arg(RX_VER_PATCHLEVEL) +
                tr("Built on " __DATE__ " at " __TIME__ "<br /><br />") +
                tr("Copyright (C) 2013-2014 Zizheng Tai ") +
                tr("<a href=\"mailto:zizheng.tai@gmail.com\">&lt;zizheng.tai@gmail.com&gt;</a><br /><br />") +
                tr("Home page: <a href=\"http://www.ramseyx.org/\">&lt;http://www.ramseyx.org/&gt;</a><br /><br />") +
                tr("This program is free software: you can redistribute it and/or modify "
                   "it under the terms of the GNU General Public License as published by "
                   "the Free Software Foundation, either version 3 of the License, or "
                   "(at your option) any later version.<br /><br />"
                   "This program is distributed in the hope that it will be useful, "
                   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
                   "GNU General Public License for more details.<br /><br />"
                   "You should have received a copy of the GNU General Public License "
                   "along with this program.  If not, see "
                   "<a href=\"http://www.gnu.org/licenses/\">&lt;http://www.gnu.org/licenses/&gt;</a>."));
}

void MainWindow::onRunOnStartupChkClicked()
{
    storeStartupSetting();
}

void MainWindow::onThreadSliderPressed()
{
    isThreadSliderPressed = true;
}

void MainWindow::onThreadSliderValueChanged(int position)
{
    ui->threadLabel->setText(QString::number(position));
    if (!isThreadSliderPressed)
    {
        theController().setMaxThreadNum(ui->threadSlider->value());
        storeThreadNumber();
        updateTasksStatus();
    }
}

void MainWindow::onThreadSliderReleased()
{
    theController().setMaxThreadNum(ui->threadSlider->value());
    storeThreadNumber();
    updateTasksStatus();
    isThreadSliderPressed = false;
}

void MainWindow::onBtnStartClicked(bool silent)
{
    if (theController().isRunning())
    {
        // Pause
        theController().pause();

        killTimer(timersRunning[std::chrono::seconds(1)]);
        killTimer(timersRunning[std::chrono::seconds(10)]);
        timersRunning.erase(std::chrono::seconds(1));
        timersRunning.erase(std::chrono::seconds(10));
        updateTasksStatus();
        ui->labelStatus->setText(tr("Status: <span style=\" font-weight:600; color:#aa0000;\">Paused</span>"));
        ui->btnStart->setText(tr("&Start!"));
        trayIcon->contextMenu()->actions().front()->setText(tr("&Start!"));

    }
    else
    {
        // Start
        if (!theController().run())
        {
            if (!silent)
            {
                if (isVisible())
                    QMessageBox::information(
                                this,
                                tr("RamseyX Client"),
                                tr("No task to run currently.") + "\t\t\n",
                                QMessageBox::Ok);
                else
                    trayIcon->showMessage(tr("RamseyX Client"),
                                          tr("No task to run currently."));
            }
            return;
        }

        timersRunning[std::chrono::seconds(10)] = startTimer(10000);
        timersRunning[std::chrono::seconds(1)] = startTimer(1000);
        ui->labelStatus->setText(tr("Status: <span style=\" font-weight:600; color:#00aa00;\">Running</span>"));
        ui->btnStart->setText(tr("&Pause"));
        trayIcon->contextMenu()->actions().front()->setText(tr("&Pause"));
    }
}

void MainWindow::onBtnAccountClicked()
{
    AccountDialog dlg(this, isAccountLocked, username, password);
    connect(&dlg, SIGNAL(lock(bool, QString, QString)), this, SLOT(onAccountLocked(bool, QString, QString)));

    ui->quitAction->setDisabled(true);
    dlg.exec();
    ui->quitAction->setEnabled(true);
}

void MainWindow::onBtnWhatsUpClicked()
{
    // Change UI status
    ui->btnWhatsUp->setDisabled(true);
    ui->btnWhatsUp->setText(tr("Updating..."));
    ui->txtWhatsUp->setText(QString());

    QMetaObject::invokeMethod(whatsUpWorker, "doWork", Qt::QueuedConnection);
    QMetaObject::invokeMethod(checkForUpdateWorker, "doWork", Qt::QueuedConnection);
}

void MainWindow::onBtnRefreshClicked()
{
    if (isRefreshing)
        return;
    // Reset thread counter
    isRefreshing = 2; // RefreshOverallThread & Refresh20Thread

    // Change UI status
    ui->tab20->setDisabled(true);
    ui->btnRefresh->setDisabled(true);
    ui->btnRefresh->setText(tr("Refreshing..."));
    for (int i = 0; i < 6; ++i)
        ui->tblOverall->item(0, i)->setText(QString());
    QIcon icon;
    QFont font;
    for (int i = 0; i < 20; ++i)
        for (int j = 0; j < 6; ++j)
        {
            ui->tbl20->item(i, j)->setText(QString());
            ui->tbl20->item(i, j)->setFont(font);
            ui->tbl20->item(i, j)->setIcon(icon);
        }

    // Create threads
    QMetaObject::invokeMethod(refreshOverallWorker, "doWork");
    emit refresh20(ui->tab20->currentIndex(), username, password);
}

void MainWindow::onTab20IndexChanged(int index)
{
    if (isRefreshing)
        return;
    isRefreshing = 1;

    // Change UI status
    QIcon icon;
    QFont font;
    for (int i = 0; i < 20; ++i)
        for (int j = 0; j < 6; ++j)
        {
            ui->tbl20->item(i, j)->setText(QString());
            ui->tbl20->item(i, j)->setFont(font);
            ui->tbl20->item(i, j)->setIcon(icon);
        }

    // Create thread
    emit refresh20(index, username, password);
}

void MainWindow::onIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            show();
            break;
        default:
            break;
    }
}

void MainWindow::onWhatsUpThreadFinished(QString s)
{
    ui->txtWhatsUp->setText(s);

    isUpdating = !isUpdating;
    if (!isUpdating)
    {
        ui->btnWhatsUp->setText(tr("&Update Now"));
        ui->btnWhatsUp->setEnabled(true);
    }
}

void MainWindow::onRefreshOverallThreadFinished(
        QString progress,
        QString users,
        QString computers,
        QString tasks,
        QString days,
        QString power)
{
    // Change UI status
    ui->tblOverall->item(0, 0)->setText(progress);
    ui->tblOverall->item(0, 1)->setText(users);
    ui->tblOverall->item(0, 2)->setText(computers);
    ui->tblOverall->item(0, 3)->setText(tasks);
    ui->tblOverall->item(0, 4)->setText(days);
    ui->tblOverall->item(0, 5)->setText(power);

    if (!--isRefreshing)
    {
        ui->btnRefresh->setText(tr("&Refresh"));
        ui->btnRefresh->setEnabled(true);
        ui->tab20->setEnabled(true);
    }
}

void MainWindow::onRefresh20ThreadFinished(QString s)
{
    if (!s.isEmpty())
    {
        if (ui->tab20->currentIndex() == 0)
        {
            for (int i = 0, section = 0; i < 20; ++i)
            {
                // Rank
                ui->tbl20->item(i, 0)->setText(QString::number(i + 1));

                // Username
                ui->tbl20->item(i, 1)->setText(s.section(' ', section, section));
                ++section;

                // Score
                ui->tbl20->item(i, 2)->setText(QString::number(s.section(' ', section, section).toDouble(), 'f', 2));
                ++section;

                // Completed tasks
                ui->tbl20->item(i, 3)->setText(s.section(' ', section, section));
                ++section;

                // Cumulative time (days)
                ui->tbl20->item(i, 4)->setText(QString::number(s.section(' ', section, section).toDouble() / 24.0 / 3600.0, 'f', 2));
                ++section;

                // Current power
                ui->tbl20->item(i, 5)->setText(QString::number(s.section(' ', section, section).toDouble(), 'f', 1));
                ++section;

                if (i == 0)
                    ui->tbl20->item(0, 0)->setIcon(QIcon(QPixmap(":/png/medal_gold")));
                else if (i == 1)
                    ui->tbl20->item(1, 0)->setIcon(QIcon(QPixmap(":/png/medal_silver")));
                else if (i == 2)
                    ui->tbl20->item(2, 0)->setIcon(QIcon(QPixmap(":/png/medal_bronze")));
            }
        }
        else
        {
            bool isUser = false;
            QFont font;
            font.setBold(true);
            for (int i = 0, section = 0; i < 20 && !s.section(' ', section, section).isEmpty(); ++i, isUser = false)
            {
                if (username == s.section(' ', section + 1, section + 1))
                    isUser = true;

                // Rank
                QString ranking = s.section(' ', section, section);
                ui->tbl20->item(i, 0)->setText(ranking);
                if (isUser)
                    ui->tbl20->item(i, 0)->setFont(font);
                if (ranking == "1")
                    ui->tbl20->item(i, 0)->setIcon(QIcon(QPixmap(":/png/medal_gold")));
                else if (ranking == "2")
                    ui->tbl20->item(i, 0)->setIcon(QIcon(QPixmap(":/png/medal_silver")));
                else if (ranking == "3")
                    ui->tbl20->item(i, 0)->setIcon(QIcon(QPixmap(":/png/medal_bronze")));
                ++section;

                // Username
                ui->tbl20->item(i, 1)->setText(s.section(' ', section, section));
                if (isUser)
                    ui->tbl20->item(i, 1)->setFont(font);
                ++section;

                // Score
                ui->tbl20->item(i, 2)->setText(QString::number(s.section(' ', section, section).toDouble(), 'f', 2));
                if (isUser)
                    ui->tbl20->item(i, 2)->setFont(font);
                ++section;

                // Completed tasks
                ui->tbl20->item(i, 3)->setText(s.section(' ', section, section));
                if (isUser)
                    ui->tbl20->item(i, 3)->setFont(font);
                ++section;

                // Cumulative time (days)
                ui->tbl20->item(i, 4)->setText(QString::number(s.section(' ', section, section).toDouble() / 24.0 / 3600.0, 'f', 2));
                if (isUser)
                    ui->tbl20->item(i, 4)->setFont(font);
                ++section;

                // Current power
                ui->tbl20->item(i, 5)->setText(QString::number(s.section(' ', section, section).toDouble(), 'f', 1));
                if (isUser)
                    ui->tbl20->item(i, 5)->setFont(font);
                ++section;
            }
        }
    }

    if (!--isRefreshing)
    {
        ui->btnRefresh->setText(tr("&Refresh"));
        ui->btnRefresh->setEnabled(true);
        ui->tab20->setEnabled(true);
    }
}

void MainWindow::onAutoUploadThreadFinished()
{
    isUploading.clear();
}

void MainWindow::onAutoDownloadThreadFinished()
{
    isDownloading.clear();
    if (!theController().isRunning())
        onBtnStartClicked(true);
}

void MainWindow::onAccountLocked(bool state, QString usr, QString pwd)
{
    setAccount(usr, pwd);
    storeAccount();
    enableUserSpecificFeatures(isAccountLocked = state);
}

void MainWindow::updateTasksStatus(bool force)
{
    if (!force && !isVisible())
        return;

    std::list<RXPRINT> running, todo, completed;

    theController().getStatus(running, todo, completed);

    QString str(tr("Last save: ") +
            QDateTime::fromTime_t(theController().getLastLog()).toString("yyyy-MM-dd hh:mm:ss") +
            "<br />");
    str += tr("Single-threaded benchmark: ") + QString::number(benchmarkInDMIPS, 'f', 1) + " DMIPS<br />";
    str += tr("Cumulative computation time: ") + QString::number(theController().getTime()) +
            tr(" secs<br />");

    // Running tasks
    str += "------------------------------------<br>" +
            tr("<b>Running task(s):") + QString::number(running.size()) + "</b><br />";
    for (const auto &t : running)
        str += "<b>-Task #" + QString::number(t.id) + "_L" +
                QString::number(t.layer) + "</b>: " +
                QString::number(t.progress, 'f', 2) + tr("%<br />&nbsp;Deadline: ") +
                QDateTime::fromTime_t(t.deadline).toString("yyyy-MM-dd hh:mm:ss") + "<br />";

    // Todo tasks
    str += "------------------------------------<br>" +
            tr("<b>Todo task(s):") + QString::number(todo.size()) + "</b><br />";
    int i = 0;
    std::list<RXPRINT>::iterator t;
    for (t = todo.begin(); i < 3 && t != todo.end(); ++i, ++t)
        str += "<b>-Task #" + QString::number(t->id) + "_L" +
                QString::number(t->layer) + "</b>: " +
                QString::number(t->progress, 'f', 2) + tr("%<br />&nbsp;Deadline: ") +
                QDateTime::fromTime_t(t->deadline).toString("yyyy-MM-dd hh:mm:ss") + "<br />";
    if (t != todo.end())
        str += tr("(") + QString::number(todo.size() - 3) + tr(" more...)<br />");

    // Completed tasks
    str += "------------------------------------<br>" +
            tr("<b>Completed task(s):") + QString::number(completed.size()) + "</b><br />";
    i = 0;
    for (t = completed.begin(); i < 3 && t != completed.end(); ++i, ++t)
        str += "<b>-Task #" + QString::number(t->id) + "_L" +
                QString::number(t->layer) + tr("</b>: 100.0%<br />&nbsp;Deadline: ") +
                QDateTime::fromTime_t(t->deadline).toString("yyyy-MM-dd hh:mm:ss") + "<br />";
    if (t != completed.end())
        str += tr("(") + QString::number(completed.size() - 3) + tr(" more...)");

    ui->txtTasksStatus->setText(str);
}
