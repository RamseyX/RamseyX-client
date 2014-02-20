#ifndef WHATSUPWORKER_H
#define WHATSUPWORKER_H

#include <QObject>
#include "RamseyXController.h"

class WhatsUpWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork()
    {
        QString content;
        switch (RamseyXController::whatsup(content))
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
