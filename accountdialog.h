#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>
#include <QThread>
#include "RamseyXController.h"
#include "validateaccountworker.h"

namespace Ui {
class AccountDialog;
}

class AccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountDialog(QWidget *parent,
            bool isAccountLocked = false,
            const QString &username = QString(),
            const QString &password = QString());
    ~AccountDialog();

    void keyPressEvent(QKeyEvent *event);

public slots:
    void onLogInButtonClicked();
    void onSignUpButtonClicked();
    void onValidateAccountThreadFinished(int errorCode);

signals:
    void validateAccount(QString username, QString password);
    void lock(bool state, QString usr, QString pwd);

private:
    Ui::AccountDialog *ui;

    bool isLocked = false;

    QThread *validateAccountThread = nullptr;
    ValidateAccountWorker *validateAccountWorker = nullptr;
};

#endif // ACCOUNTDIALOG_H
