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
#ifndef RAMSEYXCONTROLLER_H
#define RAMSEYXCONTROLLER_H

#include "ramseyxtask.h"
#include <list>
#include <mutex>
#ifdef RX_QT
#include <QObject>
#include <QString>
//#include <QtSql>
#endif

struct RXPRINT
{
    unsigned long long id = 0;
    unsigned int layer = 0;
    std::time_t deadline = 0;
    double progress = 0.0;

    explicit RXPRINT(const RXTASKINFO &info) :
        id (info.id),
        layer(info.layer),
        deadline(info.deadline),
        progress(info.layer == 1 ?
                     100.0 * info.offset / RX_LAYER1_BLOCKS_PER_TASK :
                     100.0 * info.offset / taskInfo[info.combination][RX_TASKINFO_BLOCKLENGTH])
    { }
};

#ifdef RX_QT
class RamseyXController : public QObject
{
    Q_OBJECT

signals:
    void allTasksCompleted();
    void updateTasksStatus();
    void uploadFinished();
    void downloadFinished();
#else
class RamseyXController
{
#endif

private:
    mutable std::mutex mtxAllLists;
    std::list<boost::atomic<RXTASKINFO>> runningTasks;
    std::list<boost::atomic<RXTASKINFO>> todoTasks;
    std::list<boost::atomic<RXTASKINFO>> completedTasks;

    RXFLAG threadFlags[RX_MAX_THREAD_NUM];
    std::atomic<unsigned int> threadCounter{0};

    std::atomic<bool> running{false};
    unsigned int maxThreadNum = 1;

    mutable std::atomic<std::time_t> lastLog{0};
    std::atomic<std::time_t> time{0};
    RXFLAG timerFlag;

    std::string logDir;
    std::string logFileName;

/*#ifdef RX_QT
    QSqlDatabase logDatabase;
#endif*/

private:
    RamseyXController(); // Prohibit multiple instances

public:
    ~RamseyXController();

    void threadProc(unsigned int threadID,
                    boost::atomic<RXTASKINFO> &info);
    void timerProc(double responseSecs);

    void addToCompleted(const RXTASKINFO &info);
    void addNewTask(const RXTASKINFO &info);
    bool allListsEmpty() const;
    void clearLists();
    void clearOutdated(bool force = false);
    std::time_t getLastLog() const;
    std::time_t getTime() const;

    void setLogDir(const std::string &directory);
#ifdef RX_QT
    bool openLogDatabase();
    void closeLogDatabase();
#endif
    bool writeLog(bool force = false);
    bool readLog();
    bool resetLog();

    bool run();
    void pause();
    unsigned int assignThreadID();
    void onTaskComplete(unsigned int threadID);
    void onThreadOpen();
    void setMaxThreadNum(unsigned int max);

    bool isRunning() const;
    void getStatus(
        std::list<RXPRINT> &runningPrint,
        std::list<RXPRINT> &todoPrint,
        std::list<RXPRINT> &completedPrint) const;

    // Network functions
    static int whatsUp(std::string &content);
    static int getVersion(int &major, int &minor, int &patchLevel);
    static int updateNecessaryLevel(int &major, int &minor, int &patchLevel);
    static int getProjectInfo(
            unsigned long long &numOfTasksCompleted,
            unsigned long long &numOfUsers,
            unsigned long long &numOfMachines,
            unsigned long long &numOfTasks,
            unsigned long long &time,
            double &currentPower,
            double &maxPower);
    static int getTop20(std::string &content);
    static int getMe20(
            std::string &content,
            const std::string &username,
            const std::string &password);
    static int getUserInfo(
            const std::string &strUsername,
            const std::string &strPassword,
            unsigned long long &rank,
            double &score,
            unsigned long long &numOfTasksCompleted,
            unsigned long long &time,
            unsigned long long &numOfRcmd,
            std::string &strRecommender,
            double &currentPower);
    static int updateBenchmark(
            const std::string &username,
            const std::string &password,
            const std::string &computerName,
            const std::string &CPUBrand,
            double benchmark);
    static int validateUser(
            const std::string &username,
            const std::string &password,
            unsigned long long &uid);
    static int signUp(
            const std::string &username,
            const std::string &password,
            const std::string &email,
            const std::string &recommender,
            const std::string &computerName,
            const std::string &CPUBrand);
    int getNewTask(
            const std::string &username,
            const std::string &password,
            RXTASKINFO &info);
    int fillTaskLists(
            const std::string &username,
            const std::string &password);
    int uploadOneTask(
            const std::string &username,
            const std::string &password,
            const std::string &computerName,
            const std::string &CPUBrand);
    int uploadAll(
            const std::string &username,
            const std::string &password,
            const std::string &computerName,
            const std::string &CPUBrand);
#ifdef RX_QT
    static int whatsUp(QString &content);
    static int getTop20(QString &content);
    static int getMe20(
            QString &content,
            const QString &username,
            const QString &password);
#endif

    friend RamseyXController &theController();
};

RamseyXController &theController();

#endif // RAMSEYXCONTROLLER_H
