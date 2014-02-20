#include "signupdialog.h"
#include "ui_signupdialog.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QRegExp>
#include <QToolTip>

SignUpDialog::SignUpDialog(QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::SignUpDialog),
    signUpThread(new QThread(this)),
    signUpWorker(new SignUpWorker)
{
    ui->setupUi(this);

    setFixedSize(size());

    connect(ui->submitButton, SIGNAL(clicked()), this, SLOT(onSubmitButtonClicked()));

    signUpWorker->moveToThread(signUpThread);
    connect(this, SIGNAL(signUp(QString, QString, QString, QString)),
            signUpWorker, SLOT(doWork(QString, QString, QString, QString)));
    connect(signUpWorker, SIGNAL(send(int)), this, SLOT(onSignUpThreadFinished(int)));
    connect(signUpThread, SIGNAL(finished()), signUpWorker, SLOT(deleteLater()));
    signUpThread->start();
}

SignUpDialog::~SignUpDialog()
{
    signUpThread->quit();
    signUpThread->wait();

    delete ui;
}

void SignUpDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(event);
}

void SignUpDialog::onSubmitButtonClicked()
{
    QString username(ui->usernameEdit->text());
    QString password(ui->passwordEdit->text());
    QString cpassword(ui->confirmPasswordEdit->text());
    QString email(ui->emailEdit->text().toLower());
    QString recommender(ui->recommenderEdit->text());

    // Validate fields format
    if (!QRegExp("[a-zA-Z0-9_]{1,16}").exactMatch(username))
    {
        QToolTip::showText(
                    ui->usernameEdit->mapToGlobal(QPoint()),
                    tr("Username should only contain letters, numbers and underscores,") +
                        "\n" + tr("and should have a length of 1-16 characters."),
                    this);
        return;
    }
    if (password != cpassword)
    {
        QToolTip::showText(
                    ui->confirmPasswordEdit->mapToGlobal(QPoint()),
                    tr("Passwords don\'t match."),
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
    if (!QRegExp("[0-9a-z_\\-]+(\\.[0-9a-z_\\-]+)*@([0-9a-z_\\-]+\\.)+[a-z]+").exactMatch(email))
    {
        QToolTip::showText(
                    ui->emailEdit->mapToGlobal(QPoint()),
                    tr("Invalid email address."),
                    this);
        return;
    }
    if (!recommender.isEmpty() && !QRegExp("[a-zA-Z0-9_]{1,16}").exactMatch(recommender))
    {
        QToolTip::showText(
                    ui->recommenderEdit->mapToGlobal(QPoint()),
                    tr("Recommender name should only contain letters, numbers and underscores,") +
                        "\n" + tr("and should have a length of 1-16 characters."),
                    this);
        return;
    }

    // Change UI status
    ui->usernameEdit->setDisabled(true);
    ui->passwordEdit->setDisabled(true);
    ui->confirmPasswordEdit->setDisabled(true);
    ui->emailEdit->setDisabled(true);
    ui->recommenderEdit->setDisabled(true);
    ui->submitButton->setDisabled(true);
    ui->submitButton->setText(tr("Submitting..."));

    // Create thread
    emit signUp(username, password, email, recommender);
}

void SignUpDialog::onSignUpThreadFinished(int errorCode)
{
    switch (errorCode)
    {
        case RX_ERR_SUCCESS:
            QMessageBox::information(
                        this,
                        tr("RamseyX Client"),
                        tr("Signup succeeded.") + "\t\t\n\n" +
                            ui->usernameEdit->text() +
                            tr(", you are now a member of RamseyX! Log in to join all other volunteers "
                                "and contribute your computing power to mathematics!") + "\t\t\n",
                        QMessageBox::Ok);
            break;
        case RX_ERR_CONNECTION_FAILED:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Failed to connect to server.") + "\t\t\n" +
                            tr("Please try again later.") + "\t\t\n",
                        QMessageBox::Ok);
                break;
        case RX_ERR_USER_ALREADY_EXIST:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("This username is already in use.") + "\t\t\n" +
                            tr("Please try another.") + "\t\t\n",
                        QMessageBox::Ok);
            break;
        case RX_ERR_EMAIL_ALREADY_EXIST:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("This email address is already in use.") + "\t\t\n" +
                            tr("Please try another.") + "\t\t\n",
                        QMessageBox::Ok);
            break;
        case RX_ERR_INVALID_RECOMMENDER:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("The specified recommender doesn't exist.") + "\t\t\n",
                        QMessageBox::Ok);
            break;
        default:
            QMessageBox::critical(
                        this,
                        tr("RamseyX Client"),
                        tr("Submission failed.") + "\t\t\n" +
                            tr("Please try again later.") + "\t\t\n",
                        QMessageBox::Ok);
            break;
    }

    // Change UI status
    ui->submitButton->setText(tr("&Submit"));
    ui->submitButton->setEnabled(true);
    ui->usernameEdit->setEnabled(true);
    ui->passwordEdit->setEnabled(true);
    ui->confirmPasswordEdit->setEnabled(true);
    ui->emailEdit->setEnabled(true);
    ui->recommenderEdit->setEnabled(true);
    switch (errorCode)
    {
        case RX_ERR_SUCCESS:
            close();
            break;
        case RX_ERR_USER_ALREADY_EXIST:
            ui->usernameEdit->setFocus();
            break;
        case RX_ERR_EMAIL_ALREADY_EXIST:
            ui->emailEdit->setFocus();
            break;
        case RX_ERR_INVALID_RECOMMENDER:
            ui->recommenderEdit->setFocus();
            break;
        default:
            break;
    }
}
