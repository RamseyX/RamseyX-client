#ifndef REFRESH20WORKER_H
#define REFRESH20WORKER_H

#include <QObject>
#include "RamseyXController.h"

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
