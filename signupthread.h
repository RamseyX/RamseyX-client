#ifndef SIGNUPTHREAD_H
#define SIGNUPTHREAD_H

#include "RamseyXController.h"
#include "RamseyXUtils.h"
#include <QtNetwork/QHostInfo>
#include <QThread>

class SignUpThread : public QThread
{
    Q_OBJECT

private:
    RamseyXController *controller;
    QString username;
    QString password;
    QString email;
    QString recommender;

public:
    explicit SignUpThread(
            QObject *parent,
            RamseyXController *c,
            const QString &usr,
            const QString &pwd,
            const QString &eml,
            const QString &rcmd) :
        QThread(parent),
        controller(c),
        username(usr),
        password(pwd),
        email(eml),
        recommender(rcmd)
    {
    }

    void run()
    {
        int errorCode = controller->signUp(
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

#endif // SIGNUPTHREAD_H
