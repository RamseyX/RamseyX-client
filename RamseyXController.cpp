#include "RamseyXController.h"
#include "RamseyXcURLWrapper.h"
#include "RamseyXUtils.h"
#include "BitsetIterator.h"
#include <cwchar>
#include <thread>
#include <fstream>

RamseyXController &theController()
{
    static RamseyXController controller;

    return controller;
}

RamseyXController::RamseyXController() :
    logFileName(L"RamseyX" +
                std::to_wstring(RX_VER_MAJOR) + L'.' +
                std::to_wstring(RX_VER_MINOR) + L'-' +
                RamseyXUtils::to_wstring(RX_BUILD) + L".rxd")
{
    RamseyXcURLWrapper::init();
}

RamseyXController::~RamseyXController()
{
    pause();
}

void RamseyXController::addNewTask(const RXTASKINFO &info)
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
	if (runningTasks.size() < maxThreadNum)
	{
        runningTasks.emplace_back(info);
		if (running)
		{
            std::thread t(&RamseyXController::threadProc, this, assignThreadID(),
                std::ref(runningTasks.back()));
			t.detach();
		}
	}
	else
        todoTasks.emplace_back(info);
}

void RamseyXController::addToCompleted(const RXTASKINFO &info)
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
    completedTasks.emplace_back(info);
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

void RamseyXController::clearOutdated(bool force)
{
    // Clear only the oudated tasks in runningTasks and todoTasks
    if (force)
    {
        std::lock_guard<std::mutex> lk(mtxAllLists);

        for (auto i = runningTasks.begin(); i != runningTasks.end(); ++i)
            if (std::time(nullptr) >= i->load().deadline)
                i = runningTasks.erase(i);

        for (auto i = todoTasks.begin(); i != todoTasks.end(); ++i)
            if (std::time(nullptr) >= i->load().deadline)
                i = todoTasks.erase(i);
    }
    else
    {
        if (!mtxAllLists.try_lock())
            return;
        std::lock_guard<std::mutex> lk(mtxAllLists, std::adopt_lock);

        for (auto i = runningTasks.begin(); i != runningTasks.end(); ++i)
            if (std::time(nullptr) >= i->load().deadline)
                i = runningTasks.erase(i);

        for (auto i = todoTasks.begin(); i != todoTasks.end(); ++i)
            if (std::time(nullptr) >= i->load().deadline)
                i = todoTasks.erase(i);
    }
}

std::time_t RamseyXController::getLastLog() const
{
	return lastLog;
}

std::time_t RamseyXController::getTime() const
{
	return time;
}

void RamseyXController::threadProc(unsigned int threadID,
    boost::atomic<RXTASKINFO> &info)
{
    ++threadCounter;

    RXFLAG &flag = threadFlags[threadID];
    RXTASKINFO tmp(info);
    tmp.threadID = threadID;
    info.store(tmp);

    RamseyXTask().launch(info, flag);

    flag.useFlag = false;

    --threadCounter;

    if (info.load().result != RX_LAUNCH_INCOMPLETE)
        onTaskComplete(threadID);

	// When current thread is exiting after finishing computing
    if (running)
        onThreadOpen();
}

void RamseyXController::timerProc(double responseSecs)
{
    int frequency = 1.0 / responseSecs;
    auto interval = std::chrono::milliseconds(static_cast<unsigned long long>(1000 * responseSecs));
    for (int i = 0; !timerFlag.termFlag; ++i)
    {
        std::this_thread::sleep_for(interval);
        if (i >= frequency)
        {
            i = 0;
            ++time;
        }
    }
    timerFlag.useFlag = false;
}

void RamseyXController::setLogDir(const std::wstring &directory)
{
	logDir = directory;
    if (logDir[logDir.size() - 1] != L'/' && logDir[logDir.size() - 1] != L'\\')
        logDir += L'/';
}

bool RamseyXController::writeLog(bool force) const
{
	/*
	lastLog
	time
	runningTasks.size() + todoTasks.size()
	RXTASKINFOs
	completedTasks.size()
	RXTASKINFOs
	*/
    if (force)
    {
        std::ofstream fout(RamseyXUtils::to_string(logDir + logFileName),
            std::ios_base::binary);
        if (!fout)
            return false;

        lastLog = std::time(nullptr);
        std::time_t lastLogVal = lastLog, timeVal = time;
        fout.write(reinterpret_cast<const char *>(&lastLogVal), sizeof (lastLogVal));
        fout.write(reinterpret_cast<const char *>(&timeVal), sizeof (timeVal));

        std::lock_guard<std::mutex> lk(mtxAllLists);
        // runningTasks & todoTasks
        auto size = runningTasks.size() + todoTasks.size();
        fout.write(reinterpret_cast<const char *>(&size), sizeof (size));
        for (const auto &t : runningTasks)
            fout << t;
        for (const auto &t : todoTasks)
            fout << t;

        // completedTasks
        size = completedTasks.size();
        fout.write(reinterpret_cast<const char *>(&size), sizeof (size));
        for (const auto &t : completedTasks)
            fout << t;
    }
    else
    {
        if (!mtxAllLists.try_lock())
            return false;
        std::lock_guard<std::mutex> lk(mtxAllLists, std::adopt_lock);

        std::ofstream fout(RamseyXUtils::to_string(logDir + logFileName),
            std::ios_base::binary);
        if (!fout)
            return false;

        lastLog = std::time(nullptr);
        std::time_t lastLogVal = lastLog, timeVal = time;
        fout.write(reinterpret_cast<const char *>(&lastLogVal), sizeof (lastLogVal));
        fout.write(reinterpret_cast<const char *>(&timeVal), sizeof (timeVal));

        // runningTasks & todoTasks
        auto size = runningTasks.size() + todoTasks.size();
        fout.write(reinterpret_cast<const char *>(&size), sizeof (size));
        for (const auto &t : runningTasks)
            fout << t;
        for (const auto &t : todoTasks)
            fout << t;

        // completedTasks
        size = completedTasks.size();
        fout.write(reinterpret_cast<const char *>(&size), sizeof (size));
        for (const auto &t : completedTasks)
            fout << t;
    }
	
	return true;
}

bool RamseyXController::readLog()
{
    std::ifstream fin(RamseyXUtils::to_string(logDir + logFileName),
                      std::ios_base::binary);
    if (!fin)
		return false;

	// Format error
    std::time_t lastLogVal = 0, timeVal = 0;
    if (sizeof (lastLogVal) != fin.read(reinterpret_cast<char *>(&lastLogVal), sizeof (lastLogVal)).gcount())
		return false;
    if (sizeof (timeVal) != fin.read(reinterpret_cast<char *>(&timeVal), sizeof (timeVal)).gcount())
		return false;
    lastLog = lastLogVal;
    time = timeVal;

    RXTASKINFO info;
	std::list<RXTASKINFO>::size_type size = 0;

	// runningTasks & todoTasks
    if (sizeof (size) != fin.read(reinterpret_cast<char *>(&size), sizeof (size)).gcount())
		return false;
	for (decltype(size) i = 0; i < size; i++)
        if (fin >> info)
			addNewTask(info);
        else
            return false;

	// completedTasks
    if (sizeof (size) != fin.read(reinterpret_cast<char *>(&size), sizeof (size)).gcount())
		return false;
	for (decltype(size) i = 0; i < size; i++)
        if (fin >> info)
            addToCompleted(info);
        else
            return false;

	return true;
}

bool RamseyXController::resetLog()
{
	time = 0;
	clearLists();
    return writeLog(true);
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
        std::thread t(&RamseyXController::threadProc, this, assignThreadID(),
                    std::ref(info));
		t.detach();
	}

    timerFlag.useFlag = true;
    timerFlag.termFlag = false;

    std::thread timerThread(&RamseyXController::timerProc, this, 0.2);
    timerThread.detach();

	return true;
}

void RamseyXController::pause()
{
	if (!running)
		return;

	running = false;

	for (auto &i : threadFlags)
        i.termFlag = true;
    timerFlag.termFlag = true;

    while (threadCounter)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    while (timerFlag.useFlag)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

unsigned int RamseyXController::assignThreadID()
{
    for (int i = 0; i < RX_MAX_THREAD_NUM; ++i)
        if (!threadFlags[i].useFlag)
		{
            threadFlags[i].useFlag = true;
            threadFlags[i].termFlag = false;
			return i;
		}
    return RX_MAX_THREAD_NUM;
}

void RamseyXController::onTaskComplete(unsigned int threadID)
{
	std::lock_guard<std::mutex> lk(mtxAllLists);
	
	for (auto i = runningTasks.begin(); i != runningTasks.end(); ++i)
        if (i->load().threadID == threadID)
		{
            completedTasks.emplace_back(i->load());
			runningTasks.erase(i);
			break;
		}
}

void RamseyXController::onThreadOpen()
{
    std::lock_guard<std::mutex> lk(mtxAllLists);

    if (todoTasks.empty() && runningTasks.empty())
    {
#ifdef RX_QT
        emit allTasksCompleted();
#endif
    }
    else
        while (!todoTasks.empty() && runningTasks.size() < maxThreadNum)
        {
            runningTasks.emplace_back(todoTasks.front().load());
            todoTasks.pop_front();
            if (running)
            {
                std::thread t(&RamseyXController::threadProc, this, assignThreadID(),
                    std::ref(runningTasks.back()));
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
            runningTasks.emplace_back(todoTasks.front().load());
			todoTasks.pop_front();
			if (running)
			{
                std::thread t(&RamseyXController::threadProc, this, assignThreadID(),
                    std::ref(runningTasks.back()));
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
            RXFLAG &flag = threadFlags[runningTasks.back().load().threadID];
			if (running)
			{
                flag.termFlag = true;
                while (flag.useFlag)
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
            todoTasks.emplace_front(runningTasks.back().load());
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
    if (!mtxAllLists.try_lock())
        return;
    std::lock_guard<std::mutex> lk(mtxAllLists, std::adopt_lock);

	// Required information:
	// 1. ID
	// 2. Layer
	// 3. Progress
	// 4. Deadline

	runningPrint.clear();
	todoPrint.clear();
	completedPrint.clear();

	for (const auto &t : runningTasks)
        runningPrint.emplace_back(RXPRINT(t));
	
	for (const auto &t : todoTasks)
        todoPrint.emplace_back(RXPRINT(t));

	for (const auto &t : completedTasks)
        completedPrint.emplace_back(RXPRINT(t));
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

int RamseyXController::updateNecessaryLevel(int &major, int &minor, int &patchLevel)
{
    getVersion(major, minor, patchLevel);
    if (RX_VER_MAJOR < major)
        return RX_NECESSARY;
    else if (RX_VER_MINOR < minor)
        return RX_NECESSARY;
    else if (RX_VER_PATCHLEVEL < patchLevel)
        return RX_OPTIONAL;
    else
        return RX_UNNECESSARY;
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

int RamseyXController::getMe20(std::wstring &content, const std::wstring &username, const std::wstring &password)
{
    RamseyXcURLWrapper conn;

    // Get me 20
    if (!conn.standardOpt(RX_SVR_API("get_me_20.php")) || !conn.setPost())
        return RX_ERR_GET_FAILED;

    conn.addPostField(L"username", username);
    conn.addPostField(L"password", password);

    if (!conn.execute() || conn.getErrorCode() != RX_ERR_SUCCESS)
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
        if (runningTasks.size() + todoTasks.size() >= maxThreadNum * RX_MAX_TASK_NUM_TIMES)
            return RX_ERR_TASK_LISTS_FULL;
	}

	RamseyXcURLWrapper conn;

	// Get a new task
    if (!conn.standardOpt(RX_SVR_API("get_new_task.php")) || !conn.setPost())
        return RX_ERR_GET_FAILED;

	conn.addPostField(L"username", username);
	conn.addPostField(L"password", password);
    conn.addPostField(L"major_version", std::to_wstring(RX_VER_MAJOR));
    conn.addPostField(L"minor_version", std::to_wstring(RX_VER_MINOR));

	if (!conn.execute())
        return RX_ERR_CONNECTION_FAILED;
    else if (conn.getErrorCode() != RX_ERR_SUCCESS)
		return conn.getErrorCode();

	std::wstring str(conn.getString());

    info = RXTASKINFO();
    wchar_t *end = nullptr;
    // id combinationNum block layer
    info.id = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    info.combinationNum = std::wcstoull(str.c_str(), &end, 10);
	str = str.substr(str.find(L' ') + 1);
    info.block = std::wcstoull(str.c_str(), &end, 10);
    str = str.substr(str.find(L' ') + 1);
    info.layer = std::wcstoul(str.c_str(), &end, 10);
    info.deadline = std::time(nullptr) + RX_MAX_DAYS * 24 * 3600;

    addNewTask(info);
    writeLog(true);

#ifdef RX_QT
    emit updateTasksStatus();
#endif

    return RX_ERR_SUCCESS;
}

int RamseyXController::fillTaskLists(
	const std::wstring &username,
	const std::wstring &password)
{
    // Fill runningTasks and todoTasks
    RXTASKINFO info;
    while (getNewTask(username, password, info) == RX_ERR_SUCCESS);

#ifdef RX_QT
    emit downloadFinished();
#endif

    return RX_ERR_SUCCESS;
}

int RamseyXController::uploadOneTask(
        const std::wstring &username,
        const std::wstring &password,
        const std::wstring &computerName,
        const std::wstring &CPUBrand)
{
    std::lock_guard<std::mutex> lk(mtxAllLists);
    if (completedTasks.empty())
        return RX_ERR_NO_COMPLETED_TASK;

    RamseyXcURLWrapper conn;

    // Upload the first task in completedTasks
    RXTASKINFO info(completedTasks.front());
    if (std::time(nullptr) >= info.deadline)
    {
        completedTasks.pop_front();
        return RX_ERR_TASK_OUTDATED;
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
    conn.addPostField(L"task_result", std::to_wstring(info.result));            // task_result
    if (info.layer == 1)
    {
        std::wstring spawnString(RamseyXTask::makeSpawnString(info));
        /*std::wofstream fout(QString("C:\\Users\\apple\\Desktop\\%1_%2.txt").arg(info.combinationNum).arg(info.block).toStdString());
        fout << spawnString;*/
        conn.addPostField(L"spawn_string", spawnString); // spawn_string
    }

    if (!conn.execute())
        return RX_ERR_CONNECTION_FAILED;
    int errorCode = conn.getErrorCode();
    switch (errorCode)
    {
        case RX_ERR_SUCCESS:
            time = 0;
            // Do not break - intentionally
        case RX_ERR_TASK_OUTDATED:
            completedTasks.pop_front();
            break;
        default:
            break;
    }

    return errorCode;
}

int RamseyXController::uploadAll(
	const std::wstring &username,
	const std::wstring &password,
	const std::wstring &computerName,
	const std::wstring &CPUBrand)
{

	// Upload all completed tasks
    while (true)
	{
        int errorCode = uploadOneTask(username, password, computerName, CPUBrand);
        if (errorCode != RX_ERR_SUCCESS && errorCode != RX_ERR_TASK_OUTDATED)
            break;
        writeLog();
#ifdef RX_QT
        emit updateTasksStatus();
#endif
	}

#ifdef RX_QT
    emit uploadFinished();
#endif

    return RX_ERR_SUCCESS;
}

#ifdef RX_QT
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

int RamseyXController::getTop20(QString &content)
{
    RamseyXcURLWrapper conn;

    // Get top 20
    if (!conn.standardOpt(RX_SVR_API("get_top_20.php")) || !conn.execute() ||
        conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;

    content = conn.getQString();

    return RX_ERR_SUCCESS;
}

int RamseyXController::getMe20(
        QString &content,
        const QString &username,
        const QString &password)
{
    RamseyXcURLWrapper conn;

    // Get me 20
    if (!conn.standardOpt(RX_SVR_API("get_me_20.php")) || !conn.setPost())
        return RX_ERR_GET_FAILED;

    conn.addPostField(L"username", username.toStdWString());
    conn.addPostField(L"password", password.toStdWString());

    if (!conn.execute() || conn.getErrorCode() != RX_ERR_SUCCESS)
        return RX_ERR_GET_FAILED;

    content = conn.getQString();

    return RX_ERR_SUCCESS;
}

#endif
