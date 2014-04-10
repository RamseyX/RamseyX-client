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
#ifndef CHECKFORUPDATEWORKER_H
#define CHECKFORUPDATEWORKER_H

#include <QObject>
#include "ramseyxcontroller.h"

class CheckForUpdateWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork()
    {
        int major = 0, minor = 0, patchLevel = 0;
        int level = RamseyXController::updateNecessaryLevel(major, minor, patchLevel);

        emit send(level, major, minor, patchLevel);
    }

signals:
    void send(int necessaryLevel, int major, int minor, int patchLevel);
};

#endif // CHECKFORUPDATEWORKER_H
