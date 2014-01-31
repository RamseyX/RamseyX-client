#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "RamseyXController.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void show();
    void initialize();
    void setAuto(bool state);
    void timerEvent(QTimerEvent *event);
    void setUserInfo(const QString &usr, const QString &pwd, unsigned long long uid);

public slots:
    void onBtnStartClicked();
    void onBtnAccountClicked();
    void onBtnWhatsUpClicked();
    void onBtnRefreshClicked();
    void onAccountLocked(bool state);
    void onActionAbout();
    void onWhatsUpThreadFinished(const QString &s);
    void onRefreshThreadFinished(
            const QString &progress,
            const QString &users,
            const QString &computers,
            const QString &tasks,
            const QString &days,
            const QString &power);
    void updateTasksStatus();

private:
    Ui::MainWindow *ui;

public:
    RamseyXController controller;

private:
    bool isAuto;
    bool isAccountLocked;
    QString username;
    QString password;
    unsigned long long userID;
    enum
    {
        TIMER_RUNNING_1S = 0,
        TIMER_RUNNING_10S,
        TIMER_ALWAYS_10MIN
    };
    int timers[20] = {};
};

#endif // MAINWINDOW_H
