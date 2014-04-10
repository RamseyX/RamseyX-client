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
#ifndef REFRESH20WORKER_H
#define REFRESH20WORKER_H

#include <QObject>
#include "ramseyxcontroller.h"

class Refresh20Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(int currentTabIndex, QString username, QString password)
    {
        if (currentTabIndex == 0)
        {
            // Top 20
            QString content;
            RamseyXController::getTop20(content);
            emit send(content);
        }
        else if (currentTabIndex == 1)
        {
            // Me
            QString content;
            RamseyXController::getMe20(content, username, password);
            emit send(content);
        }
    }

signals:
    void send(QString s);
};

#endif // REFRESH20WORKER_H
