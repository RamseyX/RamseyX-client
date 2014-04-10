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
#ifndef REFRESHOVERALLWORKER_H
#define REFRESHOVERALLWORKER_H

#include <QObject>
#include "ramseyxcontroller.h"

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
