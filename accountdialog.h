/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/
#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>
#include <QThread>
#include "ramseyxcontroller.h"
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
