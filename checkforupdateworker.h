#ifndef CHECKFORUPDATEWORKER_H
#define CHECKFORUPDATEWORKER_H

#include <QObject>
#include "RamseyXController.h"

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
