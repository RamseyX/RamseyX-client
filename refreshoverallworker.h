#ifndef REFRESHOVERALLWORKER_H
#define REFRESHOVERALLWORKER_H

#include <QObject>
#include "RamseyXController.h"

class RefreshOverallWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork()
    {
        unsigned long long numOfTasksCompleted = 0;
        unsigned long long numOfUsers = 0;
        unsigned long long numOfMachines = 0;
        unsigned long long numOfTasks = 0;
        unsigned long long time = 0;
        double currentPower = 0.0;
        double maxPower = 0.0;

        if (RamseyXController::getProjectInfo(numOfTasksCompleted,
                numOfUsers, numOfMachines, numOfTasks,
                time, currentPower, maxPower) != RX_ERR_SUCCESS)
        {
            emit send("", "", "", "", "", "");
            return;
        }

        double progress = numOfTasksCompleted * 100.0 / numOfTasks;
        if (100.0 - progress <= 0.01 && numOfTasksCompleted < numOfTasks)
            progress = 99.99;

        emit send(QString::number(progress, 'f', 2) + '%',
                  QString::number(numOfUsers),
                  QString::number(numOfMachines),
                  QString::number(numOfTasks),
                  QString::number(time / 24.0 / 3600.0, 'f', 2),
                  QString::number(currentPower / 1000.0, 'f', 1) +
                    " / " + QString::number(maxPower / 1000.0, 'f', 1));
    }

signals:
    void send(
            QString progress,
            QString users,
            QString computers,
            QString tasks,
            QString days,
            QString power);
};

#endif // REFRESHOVERALLWORKER_H
