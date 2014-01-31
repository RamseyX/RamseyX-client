#include "signupdialog.h"
#include "ui_signupdialog.h"
#include "signupthread.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QRegExp>
#include <QToolTip>

SignUpDialog::SignUpDialog(QWidget *parent, RamseyXController *c) :
    QDialog(parent),
    ui(new Ui::SignUpDialog),
    controller(c)
{
    ui->setupUi(this);

    connect(ui->btnSubmit, SIGNAL(clicked()), this, SLOT(onBtnSubmitClicked()));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

SignUpDialog::~SignUpDialog()
{
    delete ui;
}

void SignUpDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

void SignUpDialog::onBtnSubmitClicked()
{
    QString username(ui->lineUsername->text());
    QString password(ui->linePassword->text());
    QString cpassword(ui->lineConfirmPassword->text());
    QString email(ui->lineEmail->text().toLower());
    QString recommender(ui->lineRecommender->text());

    // Validate fields format
    if (!QRegExp("[a-zA-Z0-9_]{1,16}").exactMatch(username))
    {
        QToolTip::showText(
                    ui->lineUsername->mapToGlobal(QPoint()),
                    tr("Username should only contain letters, numbers and underscores,") +
                        "\n" + tr("and should have a length of 1-16 characters."),
                    this);
        return;
    }
    if (password != cpassword)
    {
        QToolTip::showText(
                    ui->lineConfirmPassword->mapToGlobal(QPoint()),
                    tr("Passwords don\'t match."),
                    this);
        return;
    }
    if (!QRegExp("[a-zA-Z0-9_]{4,16}").exactMatch(password))
    {
        QToolTip::showText(
                    ui->linePassword->mapToGlobal(QPoint()),
                    tr("Password should only contain letters, numbers and underscores,") +
                        "\n" + tr("and should have a length of 4-16 characters."),
                    this);
        return;
    }
    if (!QRegExp("[0-9a-z_\\-]+(\\.[0-9a-z_\\-]+)*@([0-9a-z_\\-]+\\.)+[a-z]+").exactMatch(email))
    {
        QToolTip::showText(
                    ui->lineEmail->mapToGlobal(QPoint()),
                    tr("Invalid email address."),
                    this);
        return;
    }
    if (!recommender.isEmpty() && !QRegExp("[a-zA-Z0-9_]{1,16}").exactMatch(recommender))
    {
        QToolTip::showText(
                    ui->lineRecommender->mapToGlobal(QPoint()),
                    tr("Recommender name should only contain letters, numbers and underscores,") +
                        "\n" + tr("and should have a length of 1-16 characters."),
                    this);
        return;
    }

    // Change UI status
    ui->lineUsername->setDisabled(true);
    ui->linePassword->setDisabled(true);
    ui->lineConfirmPassword->setDisabled(true);
    ui->lineEmail->setDisabled(true);
    ui->lineRecommender->setDisabled(true);
    ui->btnSubmit->setDisabled(true);
    ui->btnSubmit->setText(tr("Submitting..."));

    // Create thread
    SignUpThread *thread = new SignUpThread(
                this,
                controller,
                username,
                password,
                email,
                recommender);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(send(int)), this, SLOT(onSignUpThreadFinished(int)));
    thread->start();
}

void SignUpDialog::onSignUpThreadFinished(int errorCode)
{
    switch (errorCode)
    {
        case RX_ERR_SUCCESS:
            QMessageBox::information(
                        this,
                        tr("RamseyX Client"),
                        tr("Signup succeeded.") + "\t\n" +
                            ui->lineUsername->text() +
                            tr(", you are now a member of RamseyX! Log in to join all other volunteers "
                                "and contribute your computing power to mathematics!") + "\t\n",
                        QMessageBox::Ok);
            break;
        case RX_ERR_CONNECTION_FAILED:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Failed to connect to server.") + "\t\n" +
                            tr("Please try again later.") + "\t\n",
                        QMessageBox::Ok);
                break;
        case RX_ERR_USER_ALREADY_EXIST:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("This username is already in use.") + "\t\n" +
                            tr("Please try another.") + "\t\n",
                        QMessageBox::Ok);
            break;
        case RX_ERR_EMAIL_ALREADY_EXIST:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("This email address is already in use.") + "\t\n" +
                            tr("Please try another.") + "\t\n",
                        QMessageBox::Ok);
            break;
        case RX_ERR_INVALID_RECOMMENDER:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("The specified recommender doesn't exist.") + "\t\n",
                        QMessageBox::Ok);
            break;
        default:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Submission failed.") + "\t\n" +
                            tr("Please try again later.") + "\t\n",
                        QMessageBox::Ok);
            break;
    }

    // Change UI status
    ui->btnSubmit->setText(tr("&Submit"));
    ui->btnSubmit->setEnabled(true);
    ui->lineUsername->setEnabled(true);
    ui->linePassword->setEnabled(true);
    ui->lineConfirmPassword->setEnabled(true);
    ui->lineEmail->setEnabled(true);
    ui->lineRecommender->setEnabled(true);
    switch (errorCode)
    {
        case RX_ERR_SUCCESS:
            close();
            break;
        case RX_ERR_USER_ALREADY_EXIST:
            ui->lineUsername->setFocus();
            break;
        case RX_ERR_EMAIL_ALREADY_EXIST:
            ui->lineEmail->setFocus();
            break;
        case RX_ERR_INVALID_RECOMMENDER:
            ui->lineRecommender->setFocus();
            break;
        default:
            break;
    }
}
