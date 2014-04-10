/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>, et al.
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
#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>
#include <QThread>
#include "signupworker.h"
#include "ramseyxcontroller.h"

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
