#ifndef RX_RAMSEYXCONTROLLER_H
#define RX_RAMSEYXCONTROLLER_H

#include "RamseyXTask.h"
#include <list>
#include <mutex>
#ifdef RX_QT
#include <QObject>
#include <QString>
#endif

const int RX_MAX_THREAD_NUM = 256;

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

    std::wstring logDir;
    std::wstring logFileName;

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

	void setLogDir(const std::wstring &directory);
    bool writeLog(bool force = false) const;
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
	static int whatsup(std::wstring &content);
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
	static int getTop20(std::wstring &content);
    static int getMe20(
            std::wstring &content,
            const std::wstring &username,
            const std::wstring &password);
    static int getUserInfo(
            const std::wstring &strUsername,
            const std::wstring &strPassword,
            unsigned long long &rank,
            double &score,
            unsigned long long &numOfTasksCompleted,
            unsigned long long &time,
            unsigned long long &numOfRcmd,
            std::wstring &strRecommender,
            double &currentPower);
	static int updateBenchmark(
            const std::wstring &username,
            const std::wstring &password,
            const std::wstring &computerName,
            const std::wstring &CPUBrand,
            double benchmark);
	static int validateUser(
            const std::wstring &username,
            const std::wstring &password,
            unsigned long long &uid);
	static int signUp(
            const std::wstring &username,
            const std::wstring &password,
            const std::wstring &email,
            const std::wstring &recommender,
            const std::wstring &computerName,
            const std::wstring &CPUBrand);
	int getNewTask(
            const std::wstring &username,
            const std::wstring &password,
            RXTASKINFO &info);
	int fillTaskLists(
            const std::wstring &username,
            const std::wstring &password);
    int uploadOneTask(
            const std::wstring &username,
            const std::wstring &password,
            const std::wstring &computerName,
            const std::wstring &CPUBrand);
    int uploadAll(
            const std::wstring &username,
            const std::wstring &password,
            const std::wstring &computerName,
            const std::wstring &CPUBrand);
#ifdef RX_QT
    static int whatsup(QString &content);
    static int getTop20(QString &content);
    static int getMe20(
            QString &content,
            const QString &username,
            const QString &password);
#endif

    friend RamseyXController &theController();
};

RamseyXController &theController();

#endif
