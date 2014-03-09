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
#ifndef WHATSUPWORKER_H
#define WHATSUPWORKER_H

#include <QObject>
#include "ramseyxcontroller.h"

class WhatsUpWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork()
    {
        QString content;
        switch (RamseyXController::whatsUp(content))
        {
            case RX_ERR_SUCCESS:
                emit send(content);
                break;
            default:
                emit send("Failed to fetch from server.");
                break;
        }
    }

signals:
    void send(QString s);
};

#endif // WHATSUPWORKER_H
