#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "accountdialog.h"
#include "whatsupthread.h"
#include "refreshthread.h"
#include "RamseyXUtils.h"
#include <QDateTime>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QTableWidgetItem>
extern "C"
{
#include "dhry.h"
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isAuto(false),
    isAccountLocked(false),
    userID(0)
{
    ui->setupUi(this);

    connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(onBtnStartClicked()));
    connect(ui->btnAccount, SIGNAL(clicked()), this, SLOT(onBtnAccountClicked()));
    connect(ui->btnWhatsUp, SIGNAL(clicked()), this, SLOT(onBtnWhatsUpClicked()));
    connect(ui->btnRefresh, SIGNAL(clicked()), this, SLOT(onBtnRefreshClicked()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onActionAbout()));

    setWindowTitle(tr("RamseyX Client %1.%2.%3").arg(RX_VER_MAJOR).arg(RX_VER_MINOR).arg(RX_VER_PATCHLEVEL));

    // tblOverall
    ui->tblOverall->setRowCount(1);
    ui->tblOverall->setColumnCount(6);
    ui->tblOverall->setColumnWidth(0, 70);
    ui->tblOverall->setColumnWidth(1, 50);
    ui->tblOverall->setColumnWidth(2, 80);
    ui->tblOverall->setColumnWidth(3, 70);
    ui->tblOverall->setColumnWidth(4, 150);
    ui->tblOverall->horizontalHeader()->setStretchLastSection(true);
    ui->tblOverall->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tblOverall->setHorizontalHeaderLabels(QStringList() << tr("Progress") << tr("Users")
                                              << tr("Computers") << tr("Tasks")
                                              << tr("Cumulative Time (Days)")
                                              << tr("Current / Max Power (KDIMPS)"));
    ui->tblOverall->horizontalHeaderItem(0)->setSizeHint(
                QSize(ui->tblOverall->horizontalHeaderItem(0)->sizeHint().width(), 25));
    ui->tblOverall->setItem(0, 0, new QTableWidgetItem(""));
    ui->tblOverall->setItem(0, 1, new QTableWidgetItem(""));
    ui->tblOverall->setItem(0, 2, new QTableWidgetItem(""));
    ui->tblOverall->setItem(0, 3, new QTableWidgetItem(""));
    ui->tblOverall->setItem(0, 4, new QTableWidgetItem(""));
    ui->tblOverall->setItem(0, 5, new QTableWidgetItem(""));
    for (int i = 0; i < 6; ++i)
    {
        ui->tblOverall->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
        ui->tblOverall->item(0, i)->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    }

    // tblMe
    ui->tblMe->setRowCount(1);
    ui->tblMe->setColumnCount(6);
    ui->tblMe->setColumnWidth(0, 60);
    ui->tblMe->setColumnWidth(1, 60);
    ui->tblMe->setColumnWidth(2, 110);
    ui->tblMe->setColumnWidth(3, 150);
    ui->tblMe->setColumnWidth(4, 60);
    ui->tblMe->horizontalHeader()->setStretchLastSection(true);
    ui->tblMe->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tblMe->setHorizontalHeaderLabels(QStringList() << tr("Ranking") << tr("Credits")
                                              << tr("Completed Tasks")
                                              << tr("Cumulative Time (Days)") << tr("Referrals")
                                              << tr("Current / Max Power (DIMPS)"));
    ui->tblMe->horizontalHeaderItem(0)->setSizeHint(
                QSize(ui->tblMe->horizontalHeaderItem(0)->sizeHint().width(), 25));
    ui->tblMe->setItem(0, 0, new QTableWidgetItem(""));
    ui->tblMe->setItem(0, 1, new QTableWidgetItem(""));
    ui->tblMe->setItem(0, 2, new QTableWidgetItem(""));
    ui->tblMe->setItem(0, 3, new QTableWidgetItem(""));
    ui->tblMe->setItem(0, 4, new QTableWidgetItem(""));
    ui->tblMe->setItem(0, 5, new QTableWidgetItem(""));
    for (int i = 0; i < 6; ++i)
    {
        ui->tblMe->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
        ui->tblMe->item(0, i)->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::show()
{
    static bool firstInstance = true;

    if (firstInstance)
    {
        firstInstance = false;
        if (isAuto)
            return;
    }

    QWidget::show();
}

void MainWindow::initialize()
{
    // Set log path & suffix
    QString dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    QDir().mkpath(dir);
    controller.setLogDir(dir.toStdWString());

    // Read log
    if (!controller.readLog())
        controller.resetLog();

    // Set timers
    timers[TIMER_ALWAYS_10MIN] = startTimer(600000);

    // Update tasks status
    updateTasksStatus();

    // Update what's up
    onBtnWhatsUpClicked();

    // Refresh
    onBtnRefreshClicked();
}

void MainWindow::setAuto(bool state)
{
    isAuto = state;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    int id = event->timerId();

    if (id == timers[TIMER_RUNNING_1S])
    {
        controller.incrementTime();
        updateTasksStatus();
    }
    else if (id == timers[TIMER_RUNNING_10S])
    {
        controller.clearOutdated();
        controller.writeLog();
    }
    else if (id == timers[TIMER_ALWAYS_10MIN])
    {
        onBtnWhatsUpClicked();
    }

    QMainWindow::timerEvent(event);
}

void MainWindow::setUserInfo(const QString &usr, const QString &pwd, unsigned long long uid)
{
    username = usr;
    password = pwd;
    userID = uid;
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(
                this,
                tr("About RamseyX Client"),
                tr("<h1>RamseyX Client %1.%2.%3</h1><br />").arg(RX_VER_MAJOR).arg(RX_VER_MINOR).arg(RX_VER_PATCHLEVEL) +
                    tr("By Zizheng Tai<br /><br />") +
                    tr("Built on " __DATE__ " at " __TIME__ " using ") + RX_BUILD + "<br /><br />" +
                    tr("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, "
                        "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
                        "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. "
                        "IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, "
                        "DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR "
                        "OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR "
                        "THE USE OR OTHER DEALINGS IN THE SOFTWARE.<br /><br />"));
}

void MainWindow::onBtnStartClicked()
{
    if (controller.isRunning())
    {
        // Pause
        controller.pause();

        killTimer(timers[TIMER_RUNNING_1S]);
        killTimer(timers[TIMER_RUNNING_10S]);
        ui->labelStatus->setText(tr("Status: <span style=\" font-weight:600; color:#aa0000;\">Paused</span>"));
        ui->btnStart->setText(tr("&Start"));
    }
    else
    {
        // Start
        if (!controller.run())
        {
            QMessageBox::information(
                        this,
                        tr("RamseyX Client"),
                        tr("No task to run currently.") + "\t\t\n",
                        QMessageBox::Ok);
            return;
        }

        timers[TIMER_RUNNING_10S] = startTimer(10000);
        timers[TIMER_RUNNING_1S] = startTimer(1000);
        ui->labelStatus->setText(tr("Status: <span style=\" font-weight:600; color:#00aa00;\">Running</span>"));
        ui->btnStart->setText(tr("&Pause"));
    }
}

void MainWindow::onBtnAccountClicked()
{
    AccountDialog dlg(this, isAccountLocked, username, password);

    connect(&dlg, SIGNAL(lock(bool)), this, SLOT(onAccountLocked(bool)));

    dlg.exec();
}

void MainWindow::onBtnWhatsUpClicked()
{
    // Change UI status
    ui->btnWhatsUp->setDisabled(true);
    ui->btnWhatsUp->setText(tr("Updating..."));
    ui->txtWhatsUp->setText(QString());

    // Create thread
    WhatsUpThread *thread = new WhatsUpThread(this, &controller);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(send(QString)), this, SLOT(onWhatsUpThreadFinished(QString)));
    thread->start();
}

void MainWindow::onBtnRefreshClicked()
{
    // Change UI status
    ui->btnRefresh->setDisabled(true);
    ui->btnRefresh->setText(tr("Refreshing..."));

    // Create thread
    RefreshThread *thread = new RefreshThread(this, &controller);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(send(QString, QString, QString, QString, QString, QString)),
            this, SLOT(onRefreshThreadFinished(QString, QString, QString, QString, QString, QString)));
    thread->start();
}

void MainWindow::onWhatsUpThreadFinished(const QString &s)
{
    ui->txtWhatsUp->setText(s);
    ui->btnWhatsUp->setText(tr("&Update Now"));
    ui->btnWhatsUp->setEnabled(true);
}

void MainWindow::onRefreshThreadFinished(
        const QString &progress,
        const QString &users,
        const QString &computers,
        const QString &tasks,
        const QString &days,
        const QString &power)
{
    // Change UI status
    ui->tblOverall->item(0, 0)->setText(progress);
    ui->tblOverall->item(0, 1)->setText(users);
    ui->tblOverall->item(0, 2)->setText(computers);
    ui->tblOverall->item(0, 3)->setText(tasks);
    ui->tblOverall->item(0, 4)->setText(days);
    ui->tblOverall->item(0, 5)->setText(power);
    ui->btnRefresh->setText(tr("&Refresh"));
    ui->btnRefresh->setEnabled(true);
}

void MainWindow::onAccountLocked(bool state)
{
    isAccountLocked = state;
    if (state)
    {
        // TODO: Update user information & Hide tips
    }
    else
    {
        // TODO: Clean user information & Show tips
    }
}

void MainWindow::updateTasksStatus()
{
    static std::list<RXPRINT> running, todo, completed;
    static QString str;

    controller.getStatus(running, todo, completed);

    double DMIPS = 5000.0;
    str = tr("Last save: ") +
            QDateTime::fromTime_t(controller.getLastLog()).toString("yyyy-MM-dd hh:mm:ss") +
            "<br />";
    str += tr("Single-threaded benchmark: ") + QString::number(DMIPS, 'f', 1) + " DMIPS<br />";
    str += tr("Cumulative computation time: ") + QString::number(controller.getTime()) +
            tr(" secs<br />");

    // Running tasks
    str += "------------------------------------<br>" +
            tr("<b>Running task(s):") + QString::number(running.size()) + "</b><br />";
    for (const auto &t : running)
        str += "<b>-Task #" + QString::number(t.id) + "_L" +
                QString::number(t.layer) + "</b>: " +
                QString::number(t.progress, 'f', 2) + tr("%<br />&nbsp;Deadline: ") +
                QDateTime::fromTime_t(t.deadline).toString("yyyy-MM-dd hh:mm:ss") + "<br />";

    // Todo tasks
    str += "------------------------------------<br>" +
            tr("<b>Todo task(s):") + QString::number(todo.size()) + "</b><br />";
    int i = 0;
    std::list<RXPRINT>::iterator t;
    for (t = todo.begin(); i < 3 && t != todo.end(); ++i, ++t)
        str += "<b>-Task #" + QString::number(t->id) + "_L" +
                QString::number(t->layer) + "</b>: " +
                QString::number(t->progress, 'f', 2) + tr("%<br />&nbsp;Deadline: ") +
                QDateTime::fromTime_t(t->deadline).toString("yyyy-MM-dd hh:mm:ss") + "<br />";
    if (t != todo.end())
        str += tr("(") + QString::number(todo.size() - 3) + tr(" more...)<br />");

    // Completed tasks
    str += "------------------------------------<br>" +
            tr("<b>Completed task(s):") + QString::number(completed.size()) + "</b><br />";
    i = 0;
    for (t = completed.begin(); i < 3 && t != completed.end(); ++i, ++t)
        str += "<b>-Task #" + QString::number(t->id) + "_L" +
                QString::number(t->layer) + tr("</b>: 100.0%<br />&nbsp;Deadline: ") +
                QDateTime::fromTime_t(t->deadline).toString("yyyy-MM-dd hh:mm:ss") + "<br />";
    if (t != completed.end())
        str += tr("(") + QString::number(completed.size() - 3) + tr(" more...)");

    ui->txtTasksStatus->setText(str);
}
