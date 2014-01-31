#ifndef VALIDATEACCOUNTTHREAD_H
#define VALIDATEACCOUNTTHREAD_H

#include "RamseyXController.h"
#include <QThread>

class ValidateAccountThread : public QThread
{
    Q_OBJECT

private:
    RamseyXController *controller;
    QString username;
    QString password;

public:
    explicit ValidateAccountThread(
            QObject *parent,
            RamseyXController *c,
            const QString &usr,
            const QString &pwd) :
        QThread(parent),
        controller(c),
        username(usr),
        password(pwd)
    {
    }

    void run()
    {
        unsigned long long userID = 0;
        int errorCode = controller->validateUser(
                    username.toStdWString(),
                    password.toStdWString(),
                    userID);
        emit send(errorCode, userID);
    }

signals:
    void send(int errorCode, unsigned long long userID);
};

#endif // VALIDATEACCOUNTTHREAD_H
