#ifndef VALIDATEACCOUNTWORKER_H
#define VALIDATEACCOUNTWORKER_H

#include <QObject>
#include "RamseyXController.h"

class ValidateAccountWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(QString username, QString password)
    {
        unsigned long long userID = 0;
        int errorCode = RamseyXController::validateUser(
                    username.toStdWString(),
                    password.toStdWString(),
                    userID);
        emit send(errorCode);
    }

signals:
    void send(int errorCode);
};

#endif // VALIDATEACCOUNTWORKER_H
