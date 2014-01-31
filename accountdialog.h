#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class AccountDialog;
}

class AccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountDialog(
            QWidget *parent = 0,
            bool isAccountLocked = false,
            const QString &username = "",
            const QString &password = "");
    ~AccountDialog();

    void keyPressEvent(QKeyEvent *e);

public slots:
    void onChkLockClicked(bool checked);
    void onBtnSignUpClicked();
    void onValidateAccountThreadFinished(int errorCode, unsigned long long userID);

signals:
    void lock(bool state);

private:
    Ui::AccountDialog *ui;
    MainWindow *mainWnd;
};

#endif // ACCOUNTDIALOG_H
