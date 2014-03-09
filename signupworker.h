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
#ifndef SIGNUPWORKER_H
#define SIGNUPWORKER_H

#include <QObject>
#include <QHostInfo>
#include "ramseyxcontroller.h"
#include "ramseyxutils.h"

class SignUpWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(QString username, QString password, QString email, QString recommender)
    {
        int errorCode = RamseyXController::signUp(
                    username.toStdString(),
                    password.toStdString(),
                    email.toStdString(),
                    recommender.toStdString(),
                    QHostInfo::localHostName().toStdString(),
                    RamseyXUtils::getCpuBrandString());

        emit send(errorCode);
    }

signals:
    void send(int errorCode);
};

#endif // SIGNUPWORKER_H
