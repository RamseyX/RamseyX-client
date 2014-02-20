#ifndef SIGNUPWORKER_H
#define SIGNUPWORKER_H

#include <QObject>
#include <QHostInfo>
#include "RamseyXController.h"
#include "RamseyXUtils.h"

class SignUpWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(QString username, QString password, QString email, QString recommender)
    {
        int errorCode = RamseyXController::signUp(
                    username.toStdWString(),
                    password.toStdWString(),
                    email.toStdWString(),
                    recommender.toStdWString(),
                    QHostInfo::localHostName().toStdWString(),
                    RamseyXUtils::getCPUBrandString());

        emit send(errorCode);
    }

signals:
    void send(int errorCode);
};

#endif // SIGNUPWORKER_H
