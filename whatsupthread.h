#ifndef WHATSUPTHREAD_H
#define WHATSUPTHREAD_H

#include "RamseyXController.h"
#include <QThread>

class WhatsUpThread : public QThread
{
    Q_OBJECT

private:
    RamseyXController *controller;

public:
    explicit WhatsUpThread(QObject *parent, RamseyXController *c) :
        QThread(parent),
        controller(c)
    {
    }

    void run()
    {
        QString content;
        switch (controller->whatsup(content))
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
    void send(const QString &s);
};

#endif // WHATSUPTHREAD_H
