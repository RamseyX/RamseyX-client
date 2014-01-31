#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>
#include "RamseyXController.h"

namespace Ui {
class SignUpDialog;
}

class SignUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignUpDialog(QWidget *parent = 0, RamseyXController *c = nullptr);
    ~SignUpDialog();

public:
    void keyPressEvent(QKeyEvent *e);

public slots:
    void onBtnSubmitClicked();
    void onSignUpThreadFinished(int errorCode);

private:
    Ui::SignUpDialog *ui;

    RamseyXController *controller;
};

#endif // SIGNUPDIALOG_H
