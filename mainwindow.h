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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QThread>
#include <map>
#include <chrono>
#include <atomic>
#include "accountdialog.h"
#include "whatsupworker.h"
#include "checkforupdateworker.h"
#include "refreshoverallworker.h"
#include "refresh20worker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, bool autoState = false);
    ~MainWindow();

    void timerEvent(QTimerEvent *event);
    void closeEvent(QCloseEvent *event);

    bool isInitialized();
    bool shouldUpdateRamseyXClient();

public slots:
    void onAllTasksCompleted();
    void onThreadSliderPressed();
    void onThreadSliderValueChanged(int position);
    void onThreadSliderReleased();
    void onBtnStartClicked(bool silent = false);
    void onBtnAccountClicked();
    void onBtnWhatsUpClicked();
    void onBtnRefreshClicked();
    void onTab20IndexChanged(int index);
    void onIconActivated(QSystemTrayIcon::ActivationReason);
    void onAccountLocked(bool state, QString usr, QString pwd);
    void onResetBenchmarkAction();
    void onMailingListAction();
    void onAboutAction();
    void onRunOnStartupChkClicked();
    void onWhatsUpThreadFinished(QString s);
    void onRefreshOverallThreadFinished(
            QString progress,
            QString users,
            QString computers,
            QString tasks,
            QString days,
            QString power);
    void onRefresh20ThreadFinished(QString s);
    void onAutoUploadThreadFinished();
    void onAutoDownloadThreadFinished();
    void onCheckForUpdateThreadFinished(int necessaryLevel, int major, int minor, int patchLevel);
    void updateTasksStatus(bool force = false);

signals:
    void refresh20(int index, QString username, QString password);

private:
    void initialize();
    QIcon loadAllSizesIcons();
    void createActions();
    void createTrayIcon(const QIcon &icon);
    void createUiThreads();
    void stopUiThreads();
    void setAccount(const QString &usr, const QString &pwd);
    void storeAccount();
    bool loadAccount();
    void storeThreadNumber();
    void loadThreadNumber();
    void storeStartupSetting();
    void loadStartupSetting();
    void enableUserSpecificFeatures(bool enabled);
    void quitAndUpdate();

private:
    Ui::MainWindow *ui;

    QAction *startAction = nullptr;
    QAction *hideAction = nullptr;
    QAction *restoreAction = nullptr;

    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayIconMenu = nullptr;

    bool initialized = false;
    bool isAuto = false;
    bool isAccountLocked = false;
    bool isThreadSliderPressed = false;
    bool silentUpdate = false;
    bool updateRamseyXClient = false;
    double benchmarkInDMIPS = 0.0;
    QString username;
    QString password;
    std::map<decltype(std::chrono::seconds()), int> timersAlways; // <interval (microseconds), id>
    std::map<decltype(std::chrono::seconds()), int> timersRunning; // <interval (microseconds), id>
    std::atomic_flag isUploading;
    std::atomic_flag isDownloading;
    std::atomic<int> isRefreshing{0};
    std::atomic<bool> isUpdating{false};
    QString dir;

    QThread *whatsUpThread = nullptr;
    QThread *checkForUpdateThread = nullptr;
    QThread *refreshOverallThread = nullptr;
    QThread *refresh20Thread = nullptr;

    WhatsUpWorker *whatsUpWorker = nullptr;
    CheckForUpdateWorker *checkForUpdateWorker = nullptr;
    RefreshOverallWorker *refreshOverallWorker = nullptr;
    Refresh20Worker *refresh20Worker = nullptr;
};

#endif // MAINWINDOW_H
