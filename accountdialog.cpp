#include "accountdialog.h"
#include "ui_accountdialog.h"
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
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::AccountDialog),
    isLocked(isAccountLocked),
    validateAccountThread(new QThread(this)),
    validateAccountWorker(new ValidateAccountWorker)
{
    ui->setupUi(this);

    setFixedSize(size());

    connect(ui->logInButton, SIGNAL(clicked()), this, SLOT(onLogInButtonClicked()));
    connect(ui->signUpButton, SIGNAL(clicked()), this, SLOT(onSignUpButtonClicked()));

    validateAccountWorker->moveToThread(validateAccountThread);
    connect(this, SIGNAL(validateAccount(QString, QString)), validateAccountWorker, SLOT(doWork(QString, QString)));
    connect(validateAccountWorker, SIGNAL(send(int)), this, SLOT(onValidateAccountThreadFinished(int)));
    connect(validateAccountThread, SIGNAL(finished()), validateAccountWorker, SLOT(deleteLater()));
    validateAccountThread->start();

    if (isLocked)
    {
        ui->usernameEdit->setText(username);
        ui->passwordEdit->setText(password);
        ui->usernameEdit->setDisabled(true);
        ui->passwordEdit->setDisabled(true);
        ui->logInButton->setText(tr("&Log Out"));
    }
}

AccountDialog::~AccountDialog()
{
    validateAccountThread->quit();
    validateAccountThread->wait();

    delete ui;
}

void AccountDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(event);
}

void AccountDialog::onLogInButtonClicked()
{
    if (isLocked)
    {
        // Warn user
        if (!theController().allListsEmpty() && QMessageBox::Cancel ==
                QMessageBox::warning(
                    this,
                    tr("RamseyX Client"),
                    tr("WARNING: You have task(s) to be completed or uploaded.") + "\t\t\n" +
                        tr("Switching to another account will invalidate current task(s).") + "\t\t\n",
                    QMessageBox::Ok, QMessageBox::Cancel))
            return;

        // Change UI status
        ui->usernameEdit->setEnabled(true);
        ui->passwordEdit->setEnabled(true);
        ui->logInButton->setText(tr("&Log In"));

        // Notify MainWindow
        emit lock(isLocked = false, QString(), QString());
    }
    else
    {
        QString username(ui->usernameEdit->text());
        QString password(ui->passwordEdit->text());

        // Validate account format
        if (!QRegExp("[a-zA-Z0-9_]{1,16}").exactMatch(username))
        {
            QToolTip::showText(
                        ui->usernameEdit->mapToGlobal(QPoint()),
                        tr("Username should only contain letters, numbers and underscores,") +
                            "\n" + tr("and should have a length of 1-16 characters."),
                        this);
            return;
        }
        if (!QRegExp("[a-zA-Z0-9_]{4,16}").exactMatch(password))
        {
            QToolTip::showText(
                        ui->passwordEdit->mapToGlobal(QPoint()),
                        tr("Password should only contain letters, numbers and underscores,") +
                            "\n" + tr("and should have a length of 4-16 characters."),
                        this);
            return;
        }

        // Change UI status
        ui->usernameEdit->setDisabled(true);
        ui->passwordEdit->setDisabled(true);
        ui->logInButton->setDisabled(true);
        ui->logInButton->setText(tr("Logging In..."));

        // Create thread
        emit validateAccount(username, password);
    }
}

void AccountDialog::onSignUpButtonClicked()
{
    SignUpDialog dlg(this);
    dlg.exec();
}

void AccountDialog::onValidateAccountThreadFinished(int errorCode)
{
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
            ui->logInButton->setText(tr("&Log In"));
            ui->logInButton->setEnabled(true);
            ui->usernameEdit->setEnabled(true);
            ui->passwordEdit->setEnabled(true);
            return;
        case RX_ERR_WRONG_USR_PWD:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Username or password incorrect.") + "\t\t\n",
                        QMessageBox::Ok);
            ui->logInButton->setText(tr("&Log In"));
            ui->logInButton->setEnabled(true);
            ui->usernameEdit->setEnabled(true);
            ui->passwordEdit->setEnabled(true);
            ui->usernameEdit->setFocus();
            return;
        default:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Validation failed.") + "\t\t\n" +
                            tr("Please try again later.") + "\t\t\n",
                        QMessageBox::Ok);
            ui->logInButton->setText(tr("&Log In"));
            ui->logInButton->setEnabled(true);
            ui->usernameEdit->setEnabled(true);
            ui->passwordEdit->setEnabled(true);
            return;
    }

    // Notify MainWindow
    emit lock(isLocked = true, ui->usernameEdit->text(), ui->passwordEdit->text());

    // Change UI status
    ui->logInButton->setText(tr("&Log Out"));
    ui->logInButton->setEnabled(true);
}

