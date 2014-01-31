#include "RamseyXController.h"
#include "RamseyXcURLWrapper.h"
#include "RamseyXUtils.h"
#include "BitsetIterator.h"
#include <cwchar>
#include <thread>
#include <fstream>
#include <QDebug>

extern unsigned long long taskInfo[][3];

RamseyXController::RamseyXController() :
	maxThreadNum(1),
	lastLog(0),
    time(0),
    logFileName(L"RamseyX" +
                std::to_wstring(RX_VER_MAJOR) + L'.' +
                std::to_wstring(RX_VER_MINOR) + L'-' +
                RamseyXUtils::to_wstring(RX_BUILD) + L".rxd")
{
}

RamseyXController::~RamseyXController()
{
	pause();
	while (threadCounter)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void RamseyXController::addNewTask(const RXTASKINFO &info)
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
	if (runningTasks.size() < maxThreadNum)
	{
		runningTasks.push_back(info);
		if (running)
		{
			std::thread t(threadProc, assignThreadID(),
				std::ref(runningTasks.back()), std::ref(*this));
			t.detach();
		}
	}
	else
		todoTasks.push_back(info);
}

void RamseyXController::addToCompleted(const RXTASKINFO &info)
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
	completedTasks.push_back(info);
}

bool RamseyXController::allListsEmpty() const
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
	return runningTasks.empty() && todoTasks.empty() && completedTasks.empty();
}

void RamseyXController::clearLists()
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
	runningTasks.clear();
	todoTasks.clear();
	completedTasks.clear();
}

void RamseyXController::clearOutdated()
{
	// Clear only the oudated tasks in runningTasks and todoTasks
	std::lock_guard<std::mutex> lk(mtxAllLists);

	for (auto i = runningTasks.begin(); i != runningTasks.end(); ++i)
		if (std::time(nullptr) >= i->deadline)
			i = runningTasks.erase(i);

	for (auto i = todoTasks.begin(); i != todoTasks.end(); ++i)
		if (std::time(nullptr) >= i->deadline)
			i = todoTasks.erase(i);
}

std::time_t RamseyXController::getLastLog() const
{
	return lastLog;
}

std::time_t RamseyXController::getTime() const
{
	return time;
}

void RamseyXController::incrementTime()
{
	++time;
}

void RamseyXController::threadProc(unsigned int threadID,
	RXTASKINFO &info, RamseyXController &controller)
{
	++controller.threadCounter;

	RXFLAG &flag = controller.threadFlags[threadID];
	info.threadID = threadID;

	RamseyXTask task;
	task.t_launch(info, flag);

	*(flag.useFlag) = false;

	--controller.threadCounter;

    if (info.result != RX_LAUNCH_INCOMPLETE)
		controller.onTaskComplete(threadID);

	// When current thread is exiting after finishing computing
	if (controller.running)
		controller.onThreadOpen();
}

void RamseyXController::setLogDir(const std::wstring &directory)
{
	logDir = directory;
    if (logDir[logDir.size() - 1] != L'/' && logDir[logDir.size() - 1] != L'\\')
        logDir += L'/';
}

bool RamseyXController::writeLog() const
{
	/*
	lastLog
	time
	runningTasks.size() + todoTasks.size()
	RXTASKINFOs
	completedTasks.size()
	RXTASKINFOs
	*/
    std::ofstream fout(RamseyXUtils::to_string(logDir + logFileName),
		std::ios_base::binary | std::ios_base::trunc);
	if (!fout.is_open())
		return false;

	lastLog = std::time(nullptr);
	fout.write((const char *)(&lastLog), sizeof (lastLog));
	fout.write((const char *)(&time), sizeof (time));

	std::lock_guard<std::mutex> lk(mtxAllLists);
	// runningTasks & todoTasks
	auto size = runningTasks.size() + todoTasks.size();
	fout.write((const char *)(&size), sizeof (size));
	for (const auto &t : runningTasks)
		fout.write((const char *)(&t), sizeof (t));
	for (const auto &t : todoTasks)
		fout.write((const char *)(&t), sizeof (t));

	// completedTasks
	size = completedTasks.size();
	fout.write((const char *)(&size), sizeof (size));
	for (const auto &t : completedTasks)
		fout.write((const char *)(&t), sizeof (t));
	
	return true;
}

bool RamseyXController::writeLogWhileUploading() const
{
	/*
	lastLog
	time
	runningTasks.size() + todoTasks.size()
	RXTASKINFOs
	completedTasks.size()
	RXTASKINFOs
	*/
    std::ofstream fout(RamseyXUtils::to_string(logDir + logFileName),
		std::ios_base::binary | std::ios_base::trunc);
	if (!fout.is_open())
		return false;

	lastLog = std::time(nullptr);
	fout.write((const char *)(&lastLog), sizeof (lastLog));
	fout.write((const char *)(&time), sizeof (time));

	// runningTasks & todoTasks
	auto size = runningTasks.size() + todoTasks.size();
	fout.write((const char *)(&size), sizeof (size));
	for (const auto &t : runningTasks)
		fout.write((const char *)(&t), sizeof (t));
	for (const auto &t : todoTasks)
		fout.write((const char *)(&t), sizeof (t));

	// completedTasks
	size = completedTasks.size();
	fout.write((const char *)(&size), sizeof (size));
	for (const auto &t : completedTasks)
		fout.write((const char *)(&t), sizeof (t));

	return true;
}

bool RamseyXController::readLog()
{
    std::ifstream fin(RamseyXUtils::to_string(logDir + logFileName),
                      std::ios_base::binary);
	if (!fin.is_open())
		return false;

	// Format error
	if (sizeof (lastLog) != fin.read((char *)(&lastLog), sizeof (lastLog)).gcount())
		return false;
	if (sizeof (time) != fin.read((char *)(&time), sizeof (time)).gcount())
		return false;

	RXTASKINFO info;
	std::list<RXTASKINFO>::size_type size = 0;

	// runningTasks & todoTasks
	if (sizeof (size) != fin.read((char *)(&size), sizeof (size)).gcount())
		return false;
	for (decltype(size) i = 0; i < size; i++)
		if (sizeof (info) != fin.read((char *)(&info), sizeof (info)).gcount())
			return false;
		else
			addNewTask(info);

	// completedTasks
	if (sizeof (size) != fin.read((char *)(&size), sizeof (size)).gcount())
		return false;
	for (decltype(size) i = 0; i < size; i++)
		if (sizeof (info) != fin.read((char *)(&info), sizeof (info)).gcount())
			return false;
		else
			addToCompleted(info);

	return true;
}

bool RamseyXController::resetLog()
{
	time = 0;
	clearLists();
	return writeLog();
}

bool RamseyXController::run()
{
	if (running)
		return false;

	std::lock_guard<std::mutex> lk(mtxAllLists);
	if (runningTasks.empty())
		return false;

	running = true;
	threadCounter = 0;
	
	for (auto &info : runningTasks)
	{
		std::thread t(threadProc, assignThreadID(),
			std::ref(info), std::ref(*this));
		t.detach();
	}

	return true;
}

void RamseyXController::pause()
{
	if (!running)
		return;

	running = false;

	for (auto &i : threadFlags)
        *(i.termFlag) = true;

    while (threadCounter)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

unsigned int RamseyXController::assignThreadID()
{
    for (int i = 0; i < RX_MAX_THREAD_NUM; ++i)
		if (!*(threadFlags[i].useFlag))
		{
			*(threadFlags[i].useFlag) = true;
			*(threadFlags[i].termFlag) = false;
			return i;
		}
    return RX_MAX_THREAD_NUM;
}

void RamseyXController::onTaskComplete(unsigned int threadID)
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
	
	for (auto i = runningTasks.begin(); i != runningTasks.end(); ++i)
		if (i->threadID == threadID)
		{
			completedTasks.push_back(*i);
			runningTasks.erase(i);
			break;
		}
}

void RamseyXController::onThreadOpen()
{
	std::lock_guard<std::mutex> lk(mtxAllLists);

	while (!todoTasks.empty() && runningTasks.size() < maxThreadNum)
	{
		runningTasks.push_back(todoTasks.front());
		todoTasks.pop_front();
		if (running)
		{
			std::thread t(threadProc, assignThreadID(),
				std::ref(runningTasks.back()), std::ref(*this));
			t.detach();
		}
	}
}

void RamseyXController::setMaxThreadNum(unsigned int max)
{
	if (maxThreadNum < max)
	{
		maxThreadNum = max;
		// Move task(s) to runningTasks from todoTasks (if any)
		std::lock_guard<std::mutex> lk(mtxAllLists);
		while (!todoTasks.empty() && runningTasks.size() < maxThreadNum)
		{
			runningTasks.push_back(todoTasks.front());
			todoTasks.pop_front();
			if (running)
			{
				std::thread t(threadProc, assignThreadID(),
					std::ref(runningTasks.back()), std::ref(*this));
				t.detach();
			}
		}
	}
	else if (maxThreadNum > max)
	{
		maxThreadNum = max;
		// Move task(s) to todoTasks from runningTasks (if any)
        std::lock_guard<std::mutex> lk(mtxAllLists);
		while (!runningTasks.empty() && runningTasks.size() > maxThreadNum)
		{
			RXFLAG &flag = threadFlags[runningTasks.back().threadID];
			if (running)
			{
				*(flag.termFlag) = true;
				while (*(flag.useFlag))
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			todoTasks.push_front(runningTasks.back());
			runningTasks.pop_back();
		}
	}
}

bool RamseyXController::isRunning() const
{
	return running;
}

void RamseyXController::getStatus(
	std::list<RXPRINT> &runningPrint,
	std::list<RXPRINT> &todoPrint,
	std::list<RXPRINT> &completedPrint) const
{
	// Required information:
	// 1. ID
	// 2. Layer
	// 3. Progress
	// 4. Deadline

	RXPRINT print;
	runningPrint.clear();
	todoPrint.clear();
	completedPrint.clear();

	std::lock_guard<std::mutex> lk(mtxAllLists);
	
	for (const auto &t : runningTasks)
	{
		print.id = t.id;
		print.layer = t.layer;
		print.deadline = t.deadline;
		if (print.layer == 1)
            print.progress = 100.0 * t.offset / RX_LAYER1_BLOCKS_PER_TASK;
		else
            print.progress = 100.0 * t.offset / taskInfo[t.combinationNum][RX_TASKINFO_BLOCKLENGTH];
		runningPrint.push_back(print);
	}
	
	for (const auto &t : todoTasks)
	{
		print.id = t.id;
		print.layer = t.layer;
		print.deadline = t.deadline;
		if (print.layer == 1)
            print.progress = 100.0 * t.offset / RX_LAYER1_BLOCKS_PER_TASK;
		else
            print.progress = 100.0 * t.offset / taskInfo[t.combinationNum][RX_TASKINFO_BLOCKLENGTH];
		todoPrint.push_back(print);
	}

	for (const auto &t : completedTasks)
	{
		print.id = t.id;
		print.layer = t.layer;
		print.deadline = t.deadline;
		print.progress = 100.0;
		completedPrint.push_back(print);
	}
}

int RamseyXController::whatsup(std::wstring &content)
{
	RamseyXcURLWrapper conn;

	// What's up
    if (!conn.standardOpt(RX_SVR_API("whats_up.php")) || !conn.execute() ||
        conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;

	content = conn.getString();
	
    return RX_ERR_SUCCESS;
}

int RamseyXController::whatsup(QString &content)
{
    RamseyXcURLWrapper conn;

    // What's up
    if (!conn.standardOpt(RX_SVR_API("whats_up.php")) || !conn.execute() ||
        conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;

    content = conn.getQString();

    return RX_ERR_SUCCESS;
}

int RamseyXController::getVersion(int &major, int &minor, int &patchLevel)
{
	RamseyXcURLWrapper conn;

	// Get version
    if (!conn.standardOpt(RX_SVR_API("get_version.php")) || !conn.execute() ||
        conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;
	
	std::wstring str(conn.getString());

    wchar_t *end = nullptr;
    major = std::wcstol(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    minor = std::wcstol(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    patchLevel = std::wcstol(str.c_str(), &end, 10);

    return RX_ERR_SUCCESS;
}

int RamseyXController::getProjectInfo(
	unsigned long long &numOfTasksCompleted,
	unsigned long long &numOfUsers,
	unsigned long long &numOfMachines,
	unsigned long long &numOfTasks,
	unsigned long long &time,
	double &currentPower,
	double &maxPower)
{
	RamseyXcURLWrapper conn;

	// Get project info
    if (!conn.standardOpt(RX_SVR_API("get_project_info.php")) || !conn.execute() ||
        conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;

    std::wstring str(conn.getString());

    wchar_t *end = nullptr;
    numOfTasksCompleted = std::wcstoull(str.c_str(), &end, 10);
    str = str.substr(str.find(L' ') + 1);
    numOfUsers = std::wcstoull(str.c_str(), &end, 10);
    str = str.substr(str.find(L' ') + 1);
    numOfMachines = std::wcstoull(str.c_str(), &end, 10);
    str = str.substr(str.find(L' ') + 1);
    numOfTasks = std::wcstoull(str.c_str(), &end, 10);
    str = str.substr(str.find(L' ') + 1);
    time = std::wcstoull(str.c_str(), &end, 10);
    str = str.substr(str.find(L' ') + 1);
    currentPower = std::wcstod(str.c_str(), &end);
    str = str.substr(str.find(L' ') + 1);
    maxPower = std::wcstod(str.c_str(), &end);

    return RX_ERR_SUCCESS;
}

int RamseyXController::getTop20(std::wstring &content)
{
	RamseyXcURLWrapper conn;

	// Get top 20
    if (!conn.standardOpt(RX_SVR_API("get_top_20.php")) || !conn.execute() ||
        conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;

	content = conn.getString();

    return RX_ERR_SUCCESS;
}

int RamseyXController::getUserInfo(const std::wstring &strUsername,
    const std::wstring &strPassword,
    unsigned long long &rank,
    double &score,
    unsigned long long &numOfTasksCompleted,
    unsigned long long &time,
    unsigned long long &numOfRcmd,
    std::wstring &strRecommender,
    double &currentPower)
{
	RamseyXcURLWrapper conn;

	// Get user info
    if (!conn.standardOpt(RX_SVR_API("get_user_info.php")) || !conn.setPost())
        return RX_ERR_GET_FAILED;
	conn.addPostField(L"username", strUsername);
	conn.addPostField(L"password", strPassword);
    if (!conn.execute() || conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;

	std::wstring str(conn.getString());

    wchar_t *end = nullptr;
    rank = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    score = std::wcstod(str.c_str(), &end);
	str = str.substr(str.find(L' ') + 1);
    numOfTasksCompleted = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    time = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    numOfRcmd = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
	strRecommender = str.substr(0, str.find(L' '));
	str = str.substr(str.find(L' ') + 1);
    currentPower = std::wcstod(str.c_str(), &end);

    return RX_ERR_SUCCESS;
}

int RamseyXController::updateBenchmark(
	const std::wstring &username,
	const std::wstring &password,
	const std::wstring &computerName,
	const std::wstring &CPUBrand,
	double benchmark
	)
{
	RamseyXcURLWrapper conn;

	// Update benchmark
    if (!conn.standardOpt(RX_SVR_API("update_benchmark.php")) || !conn.setPost())
        return RX_ERR_UPLOAD_FAILED;
	
	conn.addPostField(L"username", username);
	conn.addPostField(L"password", password);
	conn.addPostField(L"computer_name", computerName);
	conn.addPostField(L"cpu_brand", CPUBrand);
	conn.addPostField(L"benchmark", std::to_wstring(benchmark));

	if (!conn.execute())
        return RX_ERR_UPLOAD_FAILED;

	return conn.getErrorCode();
}

int RamseyXController::validateUser(
	const std::wstring &username,
	const std::wstring &password,
	unsigned long long &uid)
{
	RamseyXcURLWrapper conn;

	// Validate username and password
    if (!conn.standardOpt(RX_SVR_API("validate_user.php")) || !conn.setPost())
        return RX_ERR_LOGIN_FAILED;

	conn.addPostField(L"username", username);
	conn.addPostField(L"password", password);

	if (!conn.execute())
        return RX_ERR_CONNECTION_FAILED;
    if (conn.getErrorCode() != RX_ERR_SUCCESS)
		return conn.getErrorCode();

    wchar_t *end = nullptr;
    uid = std::wcstoull(conn.getString().c_str(), &end, 10);

    return RX_ERR_SUCCESS;
}

int RamseyXController::signUp(
	const std::wstring &username,
	const std::wstring &password,
	const std::wstring &email,
	const std::wstring &recommender,
	const std::wstring &computerName,
	const std::wstring &CPUBrand)
{
	RamseyXcURLWrapper conn;

	// Sign up
    if (!conn.standardOpt(RX_SVR_API("sign_up.php")) || !conn.setPost())
        return RX_ERR_SIGNUP_FAILED;

	conn.addPostField(L"username", username);
	conn.addPostField(L"password", password);
	conn.addPostField(L"email", email);
    if (!recommender.empty())
        conn.addPostField(L"recommender", recommender);
	conn.addPostField(L"computer_name", computerName);
	conn.addPostField(L"cpu_brand", CPUBrand);

	if (!conn.execute())
        return RX_ERR_CONNECTION_FAILED;
	
	return conn.getErrorCode();
}

int RamseyXController::getNewTask(
	const std::wstring &username,
	const std::wstring &password,
	RXTASKINFO &info)
{
	{
		std::lock_guard<std::mutex> lk(mtxAllLists);
		if (runningTasks.size() + todoTasks.size() >= maxThreadNum * 16)
            return RX_ERR_TASK_LISTS_FULL;
	}

	RamseyXcURLWrapper conn;

	// Get a new task
    if (!conn.standardOpt(RX_SVR_API("get_new_task.php")) || !conn.setPost())
        return RX_ERR_GET_FAILED;

	conn.addPostField(L"username", username);
	conn.addPostField(L"password", password);
    conn.addPostField(L"p_ver", std::to_wstring(RX_VER_MAJOR));
    conn.addPostField(L"s_ver", std::to_wstring(RX_VER_MINOR));

	if (!conn.execute())
        return RX_ERR_CONNECTION_FAILED;
    else if (conn.getErrorCode() != RX_ERR_SUCCESS)
		return conn.getErrorCode();

	std::wstring str(conn.getString());

    wchar_t *end = nullptr;
    // task_id combination_num block layer [verification_code]
    info.id = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    info.combinationNum = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    info.block = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    info.layer = std::wcstoul(str.c_str(), &end, 10);
    info.offset = info.complexity = 0;
    info.deadline = std::time(nullptr) + RX_MAX_DAYS * 24 * 3600;
    info.result = RX_LAUNCH_INCOMPLETE;

	addNewTask(info);

    return RX_ERR_SUCCESS;
}

int RamseyXController::fillTaskLists(
	const std::wstring &username,
	const std::wstring &password
	)
{
	RamseyXcURLWrapper conn;

    RXTASKINFO info;
	// Fill runningTasks and todoTasks
	while (true)
	{
		{
			std::lock_guard<std::mutex> lk(mtxAllLists);
			if (runningTasks.size() + todoTasks.size() >= maxThreadNum * 16)
				break;
		}

		// Get a new task
        if (!conn.standardOpt(RX_SVR_API("get_new_task.php")) || !conn.setPost())
            return RX_ERR_GET_FAILED;

		conn.addPostField(L"username", username);
		conn.addPostField(L"password", password);
        conn.addPostField(L"p_ver", std::to_wstring(RX_VER_MAJOR));
        conn.addPostField(L"s_ver", std::to_wstring(RX_VER_MINOR));

		if (!conn.execute())
            return RX_ERR_CONNECTION_FAILED;
        else if (conn.getErrorCode() != RX_ERR_SUCCESS)
			return conn.getErrorCode();

		std::wstring str(conn.getString());

        wchar_t *end = nullptr;
		// task_id combination_num block layer [verification_code]
        info.id = std::wcstoull(str.c_str(), &end, 10);
		str = str.substr(str.find(L' ') + 1);
        info.combinationNum = std::wcstoull(str.c_str(), &end, 10);
		str = str.substr(str.find(L' ') + 1);
        info.block = std::wcstoull(str.c_str(), &end, 10);
		str = str.substr(str.find(L' ') + 1);
        info.layer = std::wcstoul(str.c_str(), &end, 10);
		//str = str.substr(str.find(L' ') + 1); // Now str stores the verification code
        info.offset = info.complexity = 0;
        info.deadline = std::time(nullptr) + RX_MAX_DAYS * 24 * 3600;
        info.result = RX_LAUNCH_INCOMPLETE;

		addNewTask(info);
	}

    return RX_ERR_SUCCESS;
}

int RamseyXController::uploadAll(
	const std::wstring &username,
	const std::wstring &password,
	const std::wstring &computerName,
	const std::wstring &CPUBrand
	)
{
	{
		std::lock_guard<std::mutex> lk(mtxAllLists);
		if (completedTasks.empty())
            return RX_ERR_NO_COMPLETED_TASK;
	}

	RamseyXcURLWrapper conn;

	// Upload all completed tasks
	RXTASKINFO info;
	while (true)
	{
		{
			std::lock_guard<std::mutex> lk(mtxAllLists);
			if (completedTasks.empty())
				break;

			info = completedTasks.front();

			if (info.deadline >= std::time(nullptr))
			{
				completedTasks.pop_front();
				writeLogWhileUploading();
				continue;
			}
		}
        if (!conn.standardOpt(RX_SVR_API("upload.php")) || !conn.setPost())
            return RX_ERR_UPLOAD_FAILED;

		conn.addPostField(L"username", username);									// username
		conn.addPostField(L"password", password);									// password
		conn.addPostField(L"task_id", std::to_wstring(info.id));					// task_id
		conn.addPostField(L"time", std::to_wstring(time));							// time
		conn.addPostField(L"complexity", std::to_wstring(info.complexity >> 2));	// complexity
		conn.addPostField(L"computer_name", computerName);							// computer_name
		conn.addPostField(L"cpu_brand", CPUBrand);									// cpu_brand
		conn.addPostField(L"layer", std::to_wstring(info.layer));					// layer
		if (info.layer == 1)
		{
			std::wstring resultBits;
            BitsetIterator<RX_LAYER1_BLOCKS_PER_TASK> iterator(info.resultBits);
			for (long long i = 0; (i = iterator.next()) >= 0;)
				resultBits += std::to_wstring(i) + L' ';
			conn.addPostField(L"result_bits", resultBits);
		}
		else
			conn.addPostField(L"task_result", std::to_wstring(info.result));		// task_result

		if (!conn.execute())
            return RX_ERR_CONNECTION_FAILED;
		switch (conn.getErrorCode())
		{
            case RX_ERR_SUCCESS:
            case RX_ERR_TASK_OUTDATED:
				break;
			default:
				return conn.getErrorCode();
		}
		
		time = 0;
		std::lock_guard<std::mutex> lk(mtxAllLists);
		for (auto i = completedTasks.begin(); i != completedTasks.end(); ++i)
			if (!std::memcmp(&(*i), &info, sizeof (RXTASKINFO)))
			{
				completedTasks.erase(i);
				writeLogWhileUploading();
				break;
			}
	}

    return RX_ERR_SUCCESS;
}
