#include "accountdialog.h"
#include "ui_accountdialog.h"
#include "validateaccountthread.h"
#include "signupdialog.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QRegExp>
#include <QToolTip>

AccountDialog::AccountDialog(
        QWidget *parent,
        bool isAccountLocked,
        const QString &username,
        const QString &password) :
    QDialog(parent),
    ui(new Ui::AccountDialog),
    mainWnd(static_cast<MainWindow *>(parent))
{
    ui->setupUi(this);

    connect(ui->chkLock, SIGNAL(clicked(bool)), this, SLOT(onChkLockClicked(bool)));
    connect(ui->btnSignUp, SIGNAL(clicked()), this, SLOT(onBtnSignUpClicked()));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (isAccountLocked)
    {
        ui->lineUsername->setText(username);
        ui->linePassword->setText(password);
        ui->lineUsername->setDisabled(true);
        ui->linePassword->setDisabled(true);
        ui->chkLock->setChecked(true);
        ui->btnUpload->setEnabled(true);
    }
}

AccountDialog::~AccountDialog()
{
    delete ui;
}

void AccountDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

void AccountDialog::onChkLockClicked(bool checked)
{
    if (checked)
    {
        QString username(ui->lineUsername->text());
        QString password(ui->linePassword->text());

        // Validate account format
        if (!QRegExp("[a-zA-Z0-9_]{1,16}").exactMatch(username))
        {
            QToolTip::showText(
                        ui->lineUsername->mapToGlobal(QPoint()),
                        tr("Username should only contain letters, numbers and underscores,") +
                            "\n" + tr("and should have a length of 1-16 characters."),
                        this);
            ui->chkLock->setChecked(false);
            return;
        }
        if (!QRegExp("[a-zA-Z0-9_]{4,16}").exactMatch(password))
        {
            QToolTip::showText(
                        ui->linePassword->mapToGlobal(QPoint()),
                        tr("Password should only contain letters, numbers and underscores,") +
                            "\n" + tr("and should have a length of 4-16 characters."),
                        this);
            ui->chkLock->setChecked(false);
            return;
        }

        // Change UI status
        ui->lineUsername->setDisabled(true);
        ui->linePassword->setDisabled(true);
        ui->btnUpload->setText(tr("Logging In..."));

        // Create thread
        ValidateAccountThread *thread = new ValidateAccountThread(this, &mainWnd->controller, username, password);
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(thread, SIGNAL(send(int, unsigned long long)), this, SLOT(onValidateAccountThreadFinished(int, unsigned long long)));
        thread->start();
    }
    else
    {
        // Warn user
        if (!mainWnd->controller.allListsEmpty() && QMessageBox::Cancel ==
                QMessageBox::warning(
                    this,
                    tr("RamseyX Client"),
                    tr("WARNING: You have task(s) to be completed or uploaded.") + "\t\t\n" +
                        tr("Switching to another account will invalidate current task(s).") + "\t\t\n",
                    QMessageBox::Ok, QMessageBox::Cancel))
        {
            ui->chkLock->setChecked(true);
            return;
        }

        // Change UI status
        ui->lineUsername->setEnabled(true);
        ui->linePassword->setEnabled(true);
        ui->btnUpload->setDisabled(true);

        // Notify MainWindow
        emit lock(false);
    }
}

void AccountDialog::onBtnSignUpClicked()
{
    SignUpDialog dlg(this, &mainWnd->controller);
    dlg.exec();
}

void AccountDialog::onValidateAccountThreadFinished(int errorCode, unsigned long long userID)
{
    // Validate account
    switch (errorCode)
    {
        case RX_ERR_SUCCESS:
            QMessageBox::information(
                        this,
                        tr("RamseyX Client"),
                        tr("Login succeeded!") + "\t\t\n",
                        QMessageBox::Ok);
            break;
        case RX_ERR_CONNECTION_FAILED:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Failed to connect to server.") + "\t\t\n" +
                            tr("Please try again later.") + "\t\t\n",
                        QMessageBox::Ok);
            ui->chkLock->setChecked(false);
            ui->btnUpload->setText(tr("&Upload Now"));
            ui->lineUsername->setEnabled(true);
            ui->linePassword->setEnabled(true);
            return;
        case RX_ERR_WRONG_USR_PWD:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Username or password incorrect.") + "\t\t\n",
                        QMessageBox::Ok);
            ui->chkLock->setChecked(false);
            ui->btnUpload->setText(tr("&Upload Now"));
            ui->lineUsername->setEnabled(true);
            ui->linePassword->setEnabled(true);
            ui->lineUsername->setFocus();
            return;
        default:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Validation failed.") + "\t\t\n" +
                            tr("Please try again later.") + "\t\t\n",
                        QMessageBox::Ok);
            ui->chkLock->setChecked(false);
            ui->btnUpload->setText(tr("&Upload Now"));
            ui->lineUsername->setEnabled(true);
            ui->linePassword->setEnabled(true);
            return;
    }

    // Store information in MainWindow members
    mainWnd->setUserInfo(ui->lineUsername->text(), ui->linePassword->text(), userID);

    // Change UI status
    ui->btnUpload->setText(tr("&Upload Now"));
    ui->btnUpload->setEnabled(true);

    // Notify MainWindow
    emit lock(true);
}

