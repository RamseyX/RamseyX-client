#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>
#include <QThread>
#include "signupworker.h"
#include "RamseyXController.h"

namespace Ui {
class SignUpDialog;
}

class SignUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignUpDialog(QWidget *parent = 0);
    ~SignUpDialog();

public:
    void keyPressEvent(QKeyEvent *event);

public slots:
    void onSubmitButtonClicked();
    void onSignUpThreadFinished(int errorCode);

signals:
    void signUp(QString username, QString password, QString email, QString recommender);

private:
    Ui::SignUpDialog *ui;

    QThread *signUpThread = nullptr;
    SignUpWorker *signUpWorker = nullptr;
};

#endif // SIGNUPDIALOG_H
