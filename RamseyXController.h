#ifndef RX_RAMSEYXCONTROLLER_H
#define RX_RAMSEYXCONTROLLER_H

#include "RamseyXTask.h"
#include <QString>
#include <list>
#include <mutex>

#define RX_MAX_THREAD_NUM 256

class RamseyXController
{
private:
	std::list<RXTASKINFO> runningTasks;
	std::list<RXTASKINFO> todoTasks;
	std::list<RXTASKINFO> completedTasks;

    RXFLAG threadFlags[RX_MAX_THREAD_NUM];
    std::atomic<unsigned int> threadCounter = {0};

	mutable std::mutex mtxAllLists;

	std::atomic<bool> running = {false};
	unsigned int maxThreadNum;

	mutable std::time_t lastLog;
	std::time_t time;

    std::wstring logDir;
    std::wstring logFileName;

public:
	RamseyXController();
	~RamseyXController();

	static void threadProc(unsigned int threadID,
		RXTASKINFO &info, RamseyXController &controller);

	void addToCompleted(const RXTASKINFO &info);
	void addNewTask(const RXTASKINFO &info);
	bool allListsEmpty() const;
	void clearLists();
	void clearOutdated();
	std::time_t getLastLog() const;
	std::time_t getTime() const;
	void incrementTime();

	void setLogDir(const std::wstring &directory);
	bool writeLog() const;
	bool writeLogWhileUploading() const; // mtxCompleted already locked
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
	static int getProjectInfo(
		unsigned long long &numOfTasksCompleted,
		unsigned long long &numOfUsers,
		unsigned long long &numOfMachines,
		unsigned long long &numOfTasks,
		unsigned long long &time,
		double &currentPower,
		double &maxPower);
	static int getTop20(std::wstring &content);
    static int getUserInfo(const std::wstring &strUsername,
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
		double benchmark
		);
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
		const std::wstring &password
		);
	int uploadAll(
		const std::wstring &username,
		const std::wstring &password,
		const std::wstring &computerName,
		const std::wstring &CPUBrand
		);
#ifdef QT_VERSION
    static int whatsup(QString &content);
#endif
};

#endif
