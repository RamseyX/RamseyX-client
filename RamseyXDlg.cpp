
// RamseyXDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "RamseyX.h"
#include "RamseyXDlg.h"
#include "afxdialogex.h"
#include "LoginDlg.h"
#include "Splash.h"
#include "RamseyXUtils.h"
extern "C"
{
#include "dhry.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT FetchProc(LPVOID lpParam)
{
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

	pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_FETCH_NOW);
	pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("获取中..."));

	std::wstring str;

	// What's up
	if (pDlg->m_controller.whatsup(str) != ERR_SUCCESS)
	{
		pDlg->GetDlgItem(IDC_WHATSUP)->SetWindowText(_T("获取动态不成功。"));
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_FETCH_NOW);
		pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("立即获取(&F)"));

		return 0;
	}
	else
		pDlg->GetDlgItem(IDC_WHATSUP)->SetWindowText(CString(str.c_str()));

	// Get version
	int P = 0, S = 0, T = 0;
	if (pDlg->m_controller.getVersion(P, S, T) != ERR_SUCCESS)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_FETCH_NOW);
		pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("立即获取(&F)"));

		return 0;
	}
	else if (PRIMARY_VERSION < P ||
		(PRIMARY_VERSION == P && (SECONDARY_VERSION < S || (SECONDARY_VERSION == S && TERTIARY_VERSION < T))))
	{
		CString strVersion;
		strVersion.Format(_T("%d.%d.%d"), P, S, T);

		TCHAR lszFilename[MAX_PATH + 2] = _T(".\\RamseyXUpdate.exe");
		SHELLEXECUTEINFO sei = {};
		sei.cbSize = sizeof (SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpFile = lszFilename;
		sei.lpVerb = _T("open");
		sei.lpDirectory = nullptr;
		sei.nShow = SW_SHOW;
		sei.lpParameters = strVersion;

		::ShellExecuteEx(&sei);

		pDlg->m_bIsUpdating = true;
		pDlg->SendMessage(WM_CLOSE);

		return 0;
	}

	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_FETCH_NOW);
	pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("立即获取(&F)"));

	return 0;
}

UINT RefreshRankProc(LPVOID lpParam)
{
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

	pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_REFRESH_RANK);
	pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新中..."));

	CListCtrl *pList = static_cast<CListCtrl *>(pDlg->GetDlgItem(IDC_RANK));
	// 清空旧列表
	pList->DeleteAllItems();

	pList = static_cast<CListCtrl *>(pDlg->GetDlgItem(IDC_ME));
	// 清空旧列表
	pList->DeleteAllItems();

	pList = static_cast<CListCtrl *>(pDlg->GetDlgItem(IDC_TOTAL));
	// 清空旧列表
	pList->DeleteAllItems();

	unsigned long long numOfTasksCompleted = 0;
	unsigned long long numOfUsers = 0;
	unsigned long long numOfMachines = 0;
	unsigned long long numOfTasks = 0;
	unsigned long long time = 0;
	double currentPower = 0.0;
	double maxPower = 0.0;
	
	if (pDlg->m_controller.getProjectInfo(numOfTasksCompleted,
		numOfUsers, numOfMachines, numOfTasks,
		time, currentPower, maxPower) != ERR_SUCCESS)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}

	double progress = numOfTasksCompleted * 100.0 / numOfTasks;
	if (100.0 - progress <= 0.01 && numOfTasksCompleted < numOfTasks)
		progress = 99.99;

	CString strProgress, strTime, strNumOfTasks, strPower;
	strProgress.Format(_T("%.2f%%"), progress);
	strTime.Format(_T("%.2f"), time / 86400.0);
	strNumOfTasks.Format(_T("%llu"), numOfTasks);
	strPower.Format(_T("%.1f/%.1f"), currentPower / 1000.0, maxPower / 1000.0);

	// 完成度 总用户数 总计算机数 总任务数 总时长(日) 当前/峰值算力(KDMIPS)
	pList->InsertItem(0, strProgress);
	pList->SetItem(0, 1, LVIF_TEXT, CString(std::to_wstring(numOfUsers).c_str()), 0, 0, 0, 0, 0);
	pList->SetItem(0, 2, LVIF_TEXT, CString(std::to_wstring(numOfMachines).c_str()), 0, 0, 0, 0, 0);
	pList->SetItem(0, 3, LVIF_TEXT, strNumOfTasks, 0, 0, 0, 0, 0);
	pList->SetItem(0, 4, LVIF_TEXT, strTime, 0, 0, 0, 0, 0);
	pList->SetItem(0, 5, LVIF_TEXT, strPower, 0, 0, 0, 0, 0);

	pList = static_cast<CListCtrl *>(pDlg->GetDlgItem(IDC_RANK));

	std::wstring str;
	if (pDlg->m_controller.getTop20(str) != ERR_SUCCESS)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}

	// 排行 用户名 积分 完成任务数 总时长(日) 当前在线算力
	CString strRankData(str.c_str());
	
	for (int i = 0; i < 20; ++i)
	{
		CString strTmp;
		TCHAR lszBuffer[MAX_PATH] = {};

		// rank
		strTmp.Format(_T("%d"), i + 1);
		pList->InsertItem(i, strTmp);

		// username
		swscanf_s(strRankData, _T("%s"), lszBuffer, sizeof (lszBuffer));
		strRankData = strRankData.Right(strRankData.GetLength() - strRankData.Find(_T(' '), 0) - 1);
		pList->SetItem(i, 1, LVIF_TEXT, lszBuffer, 0, 0, 0, 0, 0);

		// score
		swscanf_s(strRankData, _T("%s"), lszBuffer, sizeof (lszBuffer));
		strTmp.Format(_T("%.2f"), _wtof(lszBuffer));
		strRankData = strRankData.Right(strRankData.GetLength() - strRankData.Find(_T(' '), 0) - 1);
		pList->SetItem(i, 2, LVIF_TEXT, strTmp, 0, 0, 0, 0, 0);

		// number of tasks completed
		swscanf_s(strRankData, _T("%s"), lszBuffer, sizeof (lszBuffer));
		strRankData = strRankData.Right(strRankData.GetLength() - strRankData.Find(_T(' '), 0) - 1);
		pList->SetItem(i, 3, LVIF_TEXT, lszBuffer, 0, 0, 0, 0, 0);

		// time
		swscanf_s(strRankData, _T("%s"), lszBuffer, sizeof (lszBuffer));
		strTmp.Format(_T("%.2f"), _wtoi64(lszBuffer) / 86400.0);
		strRankData = strRankData.Right(strRankData.GetLength() - strRankData.Find(_T(' '), 0) - 1);
		pList->SetItem(i, 4, LVIF_TEXT, strTmp, 0, 0, 0, 0, 0);

		// current power
		swscanf_s(strRankData, _T("%s"), lszBuffer, sizeof (lszBuffer));
		strTmp.Format(_T("%.1f"), _wtof(lszBuffer));
		strRankData = strRankData.Right(strRankData.GetLength() - strRankData.Find(_T(' '), 0) - 1);
		pList->SetItem(i, 5, LVIF_TEXT, strTmp, 0, 0, 0, 0, 0);
	}

	if (!pDlg->m_bIsLocked)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}

	pList = static_cast<CListCtrl *>(pDlg->GetDlgItem(IDC_ME));

	unsigned long long rank = 0;
	unsigned long long numOfRcmd = 0;
	double score = 0.0;
	std::wstring strRecommender;

	if (pDlg->m_controller.getUserInfo(LPCTSTR(pDlg->m_strUsername),
		LPCTSTR(pDlg->m_strPassword),
		rank, score, numOfTasksCompleted, time, numOfRcmd,
		strRecommender, currentPower) != ERR_SUCCESS)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}

	CString strScore;
	strScore.Format(_T("%.2f"), score);
	strTime.Format(_T("%.2f"), time / 86400.0);
	strNumOfTasks.Format(_T("%llu"), numOfTasksCompleted);
	strPower.Format(_T("%.1f"), currentPower);

	// 排行 积分 总任务数 总时长(日) 推荐数 当前在线算力(DMIPS)
	pList->InsertItem(0, std::to_wstring(rank).c_str());
	pList->SetItem(0, 1, LVIF_TEXT, strScore, 0, 0, 0, 0, 0);
	pList->SetItem(0, 2, LVIF_TEXT, strNumOfTasks, 0, 0, 0, 0, 0);
	pList->SetItem(0, 3, LVIF_TEXT, strTime, 0, 0, 0, 0, 0);
	pList->SetItem(0, 4, LVIF_TEXT, std::to_wstring(numOfRcmd).c_str(), 0, 0, 0, 0, 0);
	pList->SetItem(0, 5, LVIF_TEXT, strPower, 0, 0, 0, 0, 0);

	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
	pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

	return 0;
}

UINT AutoUploadProc(LPVOID lpParam)
{
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());
	unsigned int nParam = reinterpret_cast<int>(lpParam);
	bool bUpload = ((nParam & 1U) != 0);
	bool bDownload = ((nParam & (1U << 1)) != 0);

	// Check version
	int P = 0, S = 0, T = 0;
	if (pDlg->m_controller.getVersion(P, S, T) != ERR_SUCCESS)
	{
		pDlg->m_bIsAutoUploading = false;
		return 0;
	}
	if (PRIMARY_VERSION < P || (PRIMARY_VERSION == P && SECONDARY_VERSION < S))
	{
		CString strVersion;
		strVersion.Format(_T("%d.%d.%d"), P, S, T);

		if (IDOK ==
			pDlg->MessageBox(_T("您的客户端版本过低，无法下载新任务，请更新至最新版程序。点击确定立刻更新至 ") +
			strVersion + _T("。"), _T("RamseyX 运算客户端"), MB_OKCANCEL | MB_ICONINFORMATION))
		{
			TCHAR lszFilename[MAX_PATH + 2] = _T(".\\RamseyXUpdate.exe");
			SHELLEXECUTEINFO sei = { 0 };
			sei.cbSize = sizeof (SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.lpFile = lszFilename;
			sei.lpVerb = _T("open");
			sei.lpDirectory = NULL;
			sei.nShow = SW_SHOW;
			sei.lpParameters = strVersion;

			::ShellExecuteEx(&sei);

			pDlg->m_bIsUpdating = true;
			pDlg->SendMessage(WM_CLOSE);

			return 0;
		}
		pDlg->m_bIsAutoUploading = false;
		return 0;
	}

	// Upload
	if (bUpload)
	{
		TCHAR lszComputerName[MAX_PATH + 2];
		DWORD dwSize = MAX_PATH;
		::GetComputerName(lszComputerName, &dwSize);

		switch (pDlg->m_controller.uploadAll(
			LPCTSTR(pDlg->m_strUsername),
			LPCTSTR(pDlg->m_strPassword),
			lszComputerName,
			RamseyXUtils::getCPUBrandString()))
		{
			case ERR_SUCCESS:
				break;
			default:
				pDlg->m_bIsAutoUploading = false;
				return 0;
		}
		pDlg->SendMessage(WM_THREADOPEN);
		pDlg->OnBnClickedRefreshRank();
	}

	// Fill task lists
	if (bDownload)
	{
		pDlg->m_controller.fillTaskLists(LPCTSTR(pDlg->m_strUsername),
			LPCTSTR(pDlg->m_strPassword));
		pDlg->SendMessage(WM_THREADOPEN);
	}

	pDlg->SendMessage(WM_UPDATESTATUS);

	if (!pDlg->m_controller.isRunning() && pDlg->m_bWasRunning)
		pDlg->OnBnClickedStart();

	pDlg->m_bIsAutoUploading = false;

	return 0;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRamseyXDlg 对话框
CRamseyXDlg::CRamseyXDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRamseyXDlg::IDD, pParent),
	m_uID(0),
	m_nCoreNum(0),
	m_nThreadNum(0),
	m_nCPULimit(0),
	m_bWasRunning(false),
	m_bIsAuto(false),
	m_bIsUpdating(false),
	m_bIsLocked(false),
	m_bIsExiting(false),
	m_hTimerQueue(nullptr),
	m_fVAX_MIPS(0.0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	memset(m_aTimerHandles, 0, sizeof (m_aTimerHandles));
}

void CRamseyXDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

CString CRamseyXDlg::GetStatus()
{
	CString str(_T("最后保存: ")
		+ CTime(m_controller.getLastLog()).Format(_T("%c"))
		+ _T("\r\n"));
	str.Format(_T("%s单线程算力：%.1f DMIPS\r\n"), str, m_fVAX_MIPS);
	if (m_controller.isRunning())
	{
		int nPos = m_controller.getTime() % 20;
		str += _T("状态：运算中 [");
		for (int i = 0; i < nPos; ++i)
			str += _T(' ');
		str += _T('●');
		for (int i = nPos + 1; i < 20; ++i)
			str += _T(' ');
		str += _T("]\r\n");
	}
	else
		str += _T("状态：暂停中 [●                   ]\r\n");
	str.Format(_T("%s运算时长：%llu秒\r\n-------------------------------------\r\n"), str, m_controller.getTime());

	std::list<RXPRINT> runningPrint, todoPrint, completedPrint;
	m_controller.getStatus(runningPrint, todoPrint, completedPrint);

	// 进行中任务
	str.Format(_T("%s进行中任务：%u\r\n"), str, runningPrint.size());
	for (const auto &t : runningPrint)
		str.Format(_T("%s●Task #%06llu_L%u：%.2f%%\r\n     截止：%s\r\n"),
			str, t.id, t.layer, t.progress, CTime(t.deadline).Format(_T("%c")));

	// 等待中任务
	str.Format(_T("%s-------------------------------------\r\n等待中任务：%d\r\n"), str, todoPrint.size());
	int i = 0;
	for (auto t = todoPrint.begin(); i < 3 && t != todoPrint.end(); ++i, ++t)
	{
		str.Format(_T("%s●Task #%06llu_L%u：%.2f%%\r\n     截止：%s\r\n"),
			str, t->id, t->layer, t->progress, CTime(t->deadline).Format(_T("%c")));
	}
	if (todoPrint.size() > 3)
		str.Format(_T("%s(%d more...)\r\n"), str, todoPrint.size() - 3);

	// 已完成任务
	str.Format(_T("%s-------------------------------------\r\n已完成任务：%d\r\n"), str, completedPrint.size());
	i = 0;
	for (auto t = completedPrint.begin(); i < 3 && t != completedPrint.end(); ++i, ++t)
	{
		str.Format(_T("%s●Task #%06llu_L%u：100.0%%\r\n     截止：%s\r\n"),
			str, t->id, t->layer, CTime(t->deadline).Format(_T("%c")));
	}
	if (completedPrint.size() > 3)
		str.Format(_T("%s(%d more...)\r\n"), str, completedPrint.size() - 3);

	return str;
}

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	CRamseyXDlg* pDlg = static_cast<CRamseyXDlg*>(AfxGetApp()->GetMainWnd());
	switch (reinterpret_cast<UINT>(lpParam))
	{
	case 1:	// Update
		pDlg->m_controller.incrementTime();
		pDlg->SendMessage(WM_UPDATESTATUS);
		break;
	case 2:	// Log
		pDlg->SendMessage(WM_WRITELOG);
		break;
	case 3:	// Fetch whatsup & Check for update & Update benchmark
		pDlg->SendMessage(WM_CLROUTDATED);
		AfxBeginThread(FetchProc, nullptr);
		if (pDlg->m_bIsLocked)
			AfxBeginThread(UpdateBenchmarkProc, nullptr);
		break;
	case 4:	// Auto upload
		pDlg->SendMessage(WM_AUTOUPLOAD);
		break;
	default:
		break;
	}
}

// 1 - succeeded
// 0 - failed
UINT_PTR CRamseyXDlg::SetTimer(UINT_PTR nIDEvent, UINT nElapse,
	void (CALLBACK* lpfnTimer)(HWND, UINT, UINT_PTR, DWORD))
{
	if (nIDEvent <= 0 || nIDEvent >= 10)
		return 0;
	if (!KillTimer(nIDEvent))
		return 0;

	if (!::CreateTimerQueueTimer(&(m_aTimerHandles[nIDEvent]), m_hTimerQueue,
		(WAITORTIMERCALLBACK)TimerRoutine, reinterpret_cast<PVOID>(nIDEvent), nElapse, nElapse, WT_EXECUTEDEFAULT))
	{
		m_aTimerHandles[nIDEvent] = nullptr;
		return 0;
	}
	return 1;
}

BOOL CRamseyXDlg::KillTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent <= 0 || nIDEvent >= 10)
		return FALSE;
	if (!m_aTimerHandles[nIDEvent])
		return TRUE;

	::DeleteTimerQueueTimer(m_hTimerQueue, m_aTimerHandles[nIDEvent], nullptr);
	m_aTimerHandles[nIDEvent] = nullptr;
	return TRUE;
}


BEGIN_MESSAGE_MAP(CRamseyXDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_START, &CRamseyXDlg::OnBnClickedStart)
	ON_WM_TIMER()
	ON_MESSAGE(WM_NI, &CRamseyXDlg::OnNI)
	ON_MESSAGE(WM_THREADOPEN, &CRamseyXDlg::OnThreadOpen)
	ON_MESSAGE(WM_UPDATESTATUS, &CRamseyXDlg::OnUpdateStatus)
	ON_MESSAGE(WM_ENABLECTRL, &CRamseyXDlg::OnEnableCtrl)
	ON_MESSAGE(WM_CLEARMYLIST, &CRamseyXDlg::OnClearMyList)
	ON_MESSAGE(WM_AUTOUPLOAD, &CRamseyXDlg::OnAutoUpload)
	ON_MESSAGE(WM_WRITELOG, &CRamseyXDlg::OnWriteLog)
	ON_MESSAGE(WM_CLROUTDATED, &CRamseyXDlg::OnClrOutdated)
	ON_COMMAND(ID_SHOW, &CRamseyXDlg::OnShow)
	ON_COMMAND(ID_EXIT, &CRamseyXDlg::OnExit)
	ON_BN_CLICKED(IDC_UPLOAD, &CRamseyXDlg::OnBnClickedUpload)
	ON_BN_CLICKED(IDC_AUTO, &CRamseyXDlg::OnBnClickedAuto)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LEVEL, &CRamseyXDlg::OnNMCustomdrawLevel)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_LEVEL, &CRamseyXDlg::OnNMReleasedcaptureLevel)
	ON_BN_CLICKED(IDC_FETCH_NOW, &CRamseyXDlg::OnBnClickedFetchNow)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_THREAD, &CRamseyXDlg::OnNMCustomdrawThread)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_THREAD, &CRamseyXDlg::OnNMReleasedcaptureThread)
	ON_BN_CLICKED(IDC_REFRESH_RANK, &CRamseyXDlg::OnBnClickedRefreshRank)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK, &CRamseyXDlg::OnNMClickSyslink)
	ON_BN_CLICKED(IDC_GET_NEW_TASK, &CRamseyXDlg::OnBnClickedGetNewTask)
END_MESSAGE_MAP()

// CRamseyXDlg 消息处理程序

BOOL CRamseyXDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	VERIFY((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	VERIFY(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		VERIFY(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_bIsAuto = (CString(AfxGetApp()->m_lpCmdLine).Find(_T("-auto")) >= 0);

	// 显示启动画面
	CSplash splash;
	if (!m_bIsAuto)
	{
		splash.Create(IDB_SPLASH);
		splash.UpdateWindow();
		splash.SendMessage(WM_INIT, FALSE);
	}

	m_hTimerQueue = ::CreateTimerQueue();

	// 设置工作目录
	TCHAR lszBuffer[MAX_PATH + 2];
	::GetModuleFileName(nullptr, lszBuffer, MAX_PATH);
	for (int i = lstrlen(lszBuffer) - 1; i >= 0 && lszBuffer[i] != _T('\\'); i--)
		lszBuffer[i] = _T('\0');
	::SetCurrentDirectory(lszBuffer);

	// 获取开机运行状态
	CRegKey reg;
	::GetModuleFileName(nullptr, lszBuffer, MAX_PATH);
	lstrcpy(lszBuffer + lstrlen(lszBuffer), _T("\" -auto"));
	TCHAR lszValue[MAX_PATH + 2];
	DWORD dwCnt;

	if (ERROR_SUCCESS == reg.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) &&
		ERROR_SUCCESS == reg.QueryStringValue(_T("RamseyX"), lszValue, &dwCnt))
	{
		if (!lstrcmp(lszValue, _T('\"') + CString(lszBuffer)))
			CheckDlgButton(IDC_AUTO, 1);
		else
		{
			reg.DeleteValue(_T("RamseyX"));
			CheckDlgButton(IDC_AUTO, 0);
		}
	}
	else
		CheckDlgButton(IDC_AUTO, 0);
	reg.Close();

	// 获取AppData目录
	::SHGetFolderPath(nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr, 0, lszBuffer);
	m_strDir = lszBuffer + CString(_T("\\RamseyX\\"));
	::CreateDirectory(m_strDir, nullptr);
	m_controller.setLogDir(lszBuffer + std::wstring(L"\\RamseyX\\"));

	// 获取CPU信息
	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	m_nCoreNum = si.dwNumberOfProcessors;
	m_nThreadNum = m_nCoreNum;

	CFile file;
	// 载入速度线程设置
	m_nCPULimit = 100;
	// 文件存在
	if (file.Open(m_strDir + _T("RamseyX.ini"), CFile::modeRead))
	{
		if (file.GetLength() == 2 * sizeof (int))
		{
			file.Read(&m_nCPULimit, sizeof (int));
			file.Read(&m_nThreadNum, sizeof (int));

			if (m_nCPULimit < 1 || m_nCPULimit > 100)
				m_nCPULimit = 100;
			if (m_nThreadNum < 1 || m_nThreadNum > m_nCoreNum)
				m_nThreadNum = m_nCoreNum;
		}
		file.Close();
	}
	m_controller.setMaxThreadNum(m_nThreadNum);
	m_nCPULimit = 100; // TODO

	// 载入本地日志
	::DeleteFile(m_strDir + _T("RamseyX.data4"));
	if (!m_controller.readLog())	// 不存在或格式错误
		m_controller.resetLog();

	// 载入账户设置
	if (file.Open(m_strDir + _T("RamseyX.ac"), CFile::modeRead))
	{
		TCHAR lszBuf[100];
		int nLen;

		VERIFY(sizeof (int) == file.Read(&nLen, sizeof (int)));
		VERIFY(sizeof (TCHAR) * nLen == file.Read(lszBuf, sizeof (TCHAR)* nLen));
		m_strUsername = lszBuf;

		VERIFY(sizeof (int) == file.Read(&nLen, sizeof (int)));
		VERIFY(sizeof (TCHAR) * nLen == file.Read(lszBuf, sizeof (TCHAR)* nLen));
		m_strPassword = lszBuf;

		VERIFY(sizeof (unsigned long long) == file.Read(&m_uID, sizeof (unsigned long long)));
		file.Close();

		SetTimer(4, 60000, nullptr);
		m_bIsLocked = true;
		GetDlgItem(IDC_ME_CAPTION)->SetWindowText(m_strUsername + _T(" 的战绩："));
		GetDlgItem(IDC_GET_NEW_TASK_TIP)->SetWindowText(_T(""));
		GetDlgItem(IDC_GET_NEW_TASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISABLE_AUTOUPLOAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISABLE_AUTODOWNLOAD)->EnableWindow(TRUE);
	}


	// 设置速度滑块
	CSliderCtrl *pSlider = static_cast<CSliderCtrl *>(GetDlgItem(IDC_LEVEL));
	pSlider->SetRange(1, 100);
	pSlider->SetPos(101 - m_nCPULimit);
	//m_limiter.SetRatio(m_nCPULimit);

	// 设置线程滑块
	pSlider = static_cast<CSliderCtrl *>(GetDlgItem(IDC_THREAD));
	pSlider->SetRange(1, m_nCoreNum);
	pSlider->SetPos(m_nCoreNum - m_nThreadNum + 1);
	CString strThread;
	strThread.Format(_T("%d"), m_nThreadNum);
	GetDlgItem(IDC_THREADINFO)->SetWindowText(strThread);

	// 设置排行
	CListCtrl *pList = static_cast<CListCtrl *>(GetDlgItem(IDC_TOTAL));

	pList->InsertColumn(0, _T("完成度"), 0, 60);
	pList->InsertColumn(1, _T("总用户数"), LVCFMT_RIGHT, 62);
	pList->InsertColumn(2, _T("总计算机数"), LVCFMT_RIGHT, 78);
	pList->InsertColumn(3, _T("总任务数"), LVCFMT_RIGHT, 70);
	pList->InsertColumn(4, _T("总时长(日)"), LVCFMT_RIGHT, 75);
	pList->InsertColumn(5, _T("当前/峰值总算力(KDMIPS)"), LVCFMT_RIGHT, 164);

	LVCOLUMN col;
	col.mask = LVCF_FMT;
	col.fmt = LVCFMT_RIGHT;
	pList->SetColumn(0, &col);
	pList->SetExtendedStyle(LVS_EX_GRIDLINES);

	pList = static_cast<CListCtrl *>(GetDlgItem(IDC_RANK));

	pList->InsertColumn(0, _T("排行"), 0, 40);
	pList->InsertColumn(1, _T("用户名"), LVCFMT_RIGHT, 100);
	pList->InsertColumn(2, _T("积分"), LVCFMT_RIGHT, 80);
	pList->InsertColumn(3, _T("完成任务数"), LVCFMT_RIGHT, 80);
	pList->InsertColumn(4, _T("总时长(日)"), LVCFMT_RIGHT, 75);
	pList->InsertColumn(5, _T("当前在线算力(DMIPS)"), LVCFMT_RIGHT, 135);

	col.mask = LVCF_FMT;
	col.fmt = LVCFMT_RIGHT;
	pList->SetColumn(0, &col);
	pList->SetExtendedStyle(LVS_EX_GRIDLINES);

	pList = static_cast<CListCtrl *>(GetDlgItem(IDC_ME));

	pList->InsertColumn(0, _T("排行"), 0, 50);
	pList->InsertColumn(1, _T("积分"), LVCFMT_RIGHT, 80);
	pList->InsertColumn(2, _T("完成任务数"), LVCFMT_RIGHT, 85);
	pList->InsertColumn(3, _T("总时长(日)"), LVCFMT_RIGHT, 90);
	pList->InsertColumn(4, _T("推荐数"), LVCFMT_RIGHT, 55);
	pList->InsertColumn(5, _T("当前在线算力(DMIPS)"), LVCFMT_RIGHT, 150);

	col.mask = LVCF_FMT;
	col.fmt = LVCFMT_RIGHT;
	pList->SetColumn(0, &col);
	pList->SetExtendedStyle(LVS_EX_GRIDLINES);

	// 初始化RamseyXTask静态数据
	RamseyXTask::init();

	// 基准测试
	if (!m_bIsAuto)
	{
		splash.SendMessage(WM_INIT, TRUE);
		splash.SendMessage(WM_BENCHMARK, FALSE);
	}
	m_fVAX_MIPS = 5000.0; //::Benchmark();
	
	//RamseyXTask task;
	//task.printSQLScript();

	if (!m_bIsAuto)
		splash.SendMessage(WM_BENCHMARK, TRUE);

	if (!m_bIsLocked)
		pList->EnableWindow(FALSE);

	// 获取动态
	SetTimer(3, 600000, nullptr);

	// 立即获取动态
	OnBnClickedFetchNow();

	// 立即刷新排行
	OnBnClickedRefreshRank();

	// 加满线程
	SendMessage(WM_THREADOPEN);

	// 调整线程优先级
	::SetPriorityClass(::GetCurrentProcess(), IDLE_PRIORITY_CLASS);

	// 自动运行
	if (m_bIsAuto)
		OnBnClickedStart();

	// 设置系统托盘
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof (NOTIFYICONDATA);
	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	nid.hWnd = m_hWnd;
	nid.uID = IDR_MAINFRAME;
	lstrcpy(nid.szTip, _T("RamseyX 运算客户端"));
	nid.dwInfoFlags = NIIF_INFO;
	nid.uCallbackMessage = WM_NI;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	if (!m_bIsAuto)
	{
		lstrcpy(nid.szInfoTitle, _T("RamseyX 运算客户端"));
		lstrcpy(nid.szInfo, _T("最小化主窗口可自动转入后台运行。"));
		nid.uFlags |= NIF_INFO;
	}
	::Shell_NotifyIcon(NIM_ADD, &nid);

	AfxMessageBox(RamseyXUtils::getCPUBrandString().c_str());

	RXTASKINFO info;
	info.block = info.combinationNum = info.complexity = info.id = info.offset = 0;
	info.result = LAUNCH_INCOMPLETE;
	info.deadline = std::time(nullptr) + MAX_DAYS * 24 * 3600;
	info.layer = 1;
	m_controller.addNewTask(info);
	/*++info.id;
	m_controller.addNewTask(info);
	++info.id;
	m_controller.addNewTask(info);
	++info.id;
	m_controller.addNewTask(info);
	++info.id;
	m_controller.addNewTask(info);
	++info.id;
	m_controller.addNewTask(info);*/

	// 刷新状态
	GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());
	SendMessage(WM_WRITELOG);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRamseyXDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_MINIMIZE)
	{
		ShowWindow(SW_HIDE);
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRamseyXDlg::OnPaint()
{
	static bool bIsFirst = true;
	if (m_bIsAuto && bIsFirst)
	{
		bIsFirst = false;
		ShowWindow(SW_HIDE);
	}
	else if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
		CDialogEx::OnPaint();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRamseyXDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRamseyXDlg::OnClose()
{
	if (!m_bIsUpdating && MessageBox(_T("确定要退出 RamseyX 运算客户端吗？"), _T("RamseyX 运算客户端"), MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL)
		return;

	m_controller.pause();

	NOTIFYICONDATA nid;
	nid.cbSize = sizeof (NOTIFYICONDATA);
	nid.hWnd = m_hWnd;
	nid.uID = IDR_MAINFRAME;
	::Shell_NotifyIcon(NIM_DELETE, &nid);

	::DeleteTimerQueue(m_hTimerQueue);

	CDialogEx::OnClose();
}

void CRamseyXDlg::OnBnClickedStart()
{
	if (m_controller.isRunning())
	{
		m_controller.pause();

		KillTimer(1);
		KillTimer(2);
		m_bWasRunning = false;
		GetDlgItem(IDC_START)->SetWindowText(_T("开始运算(&S)"));
		GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());
	}
	else
	{
		if (!m_controller.run())
		{
			MessageBox(_T("当前没有可进行的任务。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
			return;
		}

		SetTimer(1, 1000, nullptr);
		SetTimer(2, 10000, nullptr);
		m_bWasRunning = true;
		GetDlgItem(IDC_START)->SetWindowText(_T("暂停运算(&S)"));
		GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());
	}
}

void CRamseyXDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CRamseyXDlg::OnNI(WPARAM wParam, LPARAM lParam)
{
	CPoint pt;
	CMenu menu;

	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:
		ShowWindow(SW_SHOW);
		break;
	case WM_RBUTTONUP:
		SetForegroundWindow();
		::GetCursorPos(&pt);
		menu.LoadMenu(IDR_MENU_NI);
		if (m_controller.isRunning())
			menu.GetSubMenu(0)->ModifyMenu(IDC_START, MF_BYCOMMAND, IDC_START, _T("暂停运算(&S)"));
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTALIGN, pt.x, pt.y, this);
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CRamseyXDlg::OnThreadOpen(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CRamseyXDlg::OnShow()
{
	ShowWindow(SW_SHOW);
}

void CRamseyXDlg::OnExit()
{
	SendMessage(WM_CLOSE);
}

void CRamseyXDlg::OnBnClickedUpload()
{
	// Show login dialog
	CLoginDlg dlg;
	dlg.DoModal();
}

void CRamseyXDlg::OnBnClickedAuto()
{
	CRegKey reg;
	VERIFY(ERROR_SUCCESS == reg.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")));
	if (IsDlgButtonChecked(IDC_AUTO) == 1)
	{
		TCHAR lszBuffer[MAX_PATH + 2];
		::GetModuleFileName(nullptr, lszBuffer, MAX_PATH);
		lstrcpy(lszBuffer + lstrlen(lszBuffer), _T("\" -auto"));
		VERIFY(ERROR_SUCCESS == reg.SetStringValue(_T("RamseyX"), _T('\"') + CString(lszBuffer)));
	}
	else if (IsDlgButtonChecked(IDC_AUTO) == 0)
		reg.DeleteValue(_T("RamseyX"));
	reg.Close();
}

void CRamseyXDlg::StoreLevelSpeed() const
{
	CFile file;
	if (file.Open(m_strDir + _T("RamseyX.ini"), CFile::modeCreate | CFile::modeWrite))
	{
		file.Write(&m_nCPULimit, sizeof (int));
		file.Write(&m_nThreadNum, sizeof (int));
		file.Close();
	}
}

void CRamseyXDlg::StoreAccount() const
{
	CFile file;
	::DeleteFile(m_strDir + _T("RamseyX.ac"));
	if (m_bIsLocked)
	if (file.Open(m_strDir + _T("RamseyX.ac"), CFile::modeCreate | CFile::modeWrite))
	{
		TCHAR lszBuffer[100];

		lstrcpy(lszBuffer, m_strUsername);
		int nLen = lstrlen(lszBuffer) + 1;
		file.Write(&nLen, sizeof (int));
		file.Write(lszBuffer, sizeof (TCHAR)* nLen);

		lstrcpy(lszBuffer, m_strPassword);
		nLen = lstrlen(lszBuffer) + 1;
		file.Write(&nLen, sizeof (int));
		file.Write(lszBuffer, sizeof (TCHAR)* nLen);

		file.Write(&m_uID, sizeof (unsigned long long));
		file.Close();
	}
}

void CRamseyXDlg::OnNMCustomdrawLevel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CString strLimit;
	strLimit.Format(_T("%d%%"), 101 - static_cast<CSliderCtrl*>(GetDlgItem(IDC_LEVEL))->GetPos());
	GetDlgItem(IDC_LEVELINFO)->SetWindowText(strLimit);

	*pResult = 0;
}

void CRamseyXDlg::OnNMReleasedcaptureLevel(NMHDR *pNMHDR, LRESULT *pResult)
{
	//m_nCPULimit = 101 - static_cast<CSliderCtrl*>(GetDlgItem(IDC_LEVEL))->GetPos();
	//m_limiter.SetRatio(m_nCPULimit);
	StoreLevelSpeed();

	*pResult = 0;
}

BOOL CRamseyXDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CRamseyXDlg::OnBnClickedFetchNow()
{
	AfxBeginThread(FetchProc, nullptr);
}

void CRamseyXDlg::OnNMCustomdrawThread(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CString strThread;
	strThread.Format(_T("%d"), m_nCoreNum - static_cast<CSliderCtrl*>(GetDlgItem(IDC_THREAD))->GetPos() + 1);
	GetDlgItem(IDC_THREADINFO)->SetWindowText(strThread);

	*pResult = 0;
}

void CRamseyXDlg::OnNMReleasedcaptureThread(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_nThreadNum = m_nCoreNum - static_cast<CSliderCtrl*>(GetDlgItem(IDC_THREAD))->GetPos() + 1;

	m_controller.setMaxThreadNum(m_nThreadNum);

	m_controller.writeLog();
	StoreLevelSpeed();
	GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());

	*pResult = 0;
}

void CRamseyXDlg::OnBnClickedRefreshRank()
{
	AfxBeginThread(RefreshRankProc, nullptr);
}

void CRamseyXDlg::OnNMClickSyslink(NMHDR *pNMHDR, LRESULT *pResult)
{
	::ShellExecute(nullptr, _T("open"), _T("http://www.ramseyx.org/"), nullptr, nullptr, SW_SHOW);
	*pResult = 0;
}

#define GNT_END { pDlg->GetDlgItem(IDC_GET_NEW_TASK)->SetWindowText(_T("下载新任务(&N)")); \
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_GET_NEW_TASK); \
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTOUPLOAD); \
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTODOWNLOAD); \
	return 0; }

UINT GetNewTaskProc(LPVOID lpParam)
{
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

	pDlg->GetDlgItem(IDC_GET_NEW_TASK)->SetWindowText(_T("下载中..."));
	pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_GET_NEW_TASK);
	pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_DISABLE_AUTOUPLOAD);
	pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_DISABLE_AUTODOWNLOAD);

	// Check for update
	int P = 0, S = 0, T = 0;
	if (pDlg->m_controller.getVersion(P, S, T) != ERR_SUCCESS)
	{
		pDlg->MessageBox(_T("版本验证失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		GNT_END
	}
	if (PRIMARY_VERSION < P || (PRIMARY_VERSION == P && SECONDARY_VERSION < S))
	{
		CString strVersion;
		strVersion.Format(_T("%d.%d.%d"), P, S, T);

		if (IDOK == 
			pDlg->MessageBox(_T("您的客户端版本过低，无法下载新任务，请更新至最新版程序。点击确定立刻更新至 ") +
			strVersion + _T("。"), _T("RamseyX 运算客户端"), MB_OKCANCEL | MB_ICONINFORMATION))
		{
			TCHAR lszFilename[MAX_PATH + 2] = _T(".\\RamseyXUpdate.exe");
			SHELLEXECUTEINFO sei = { 0 };
			sei.cbSize = sizeof (SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.lpFile = lszFilename;
			sei.lpVerb = _T("open");
			sei.lpDirectory = NULL;
			sei.nShow = SW_SHOW;
			sei.lpParameters = strVersion;

			::ShellExecuteEx(&sei);

			pDlg->m_bIsUpdating = true;
			pDlg->SendMessage(WM_CLOSE);

			return 0;
		}
		else
			GNT_END
	}

	// Get a new task
	RXTASKINFO info;
	switch (pDlg->m_controller.getNewTask(
		LPCTSTR(pDlg->m_strUsername),
		LPCTSTR(pDlg->m_strPassword),
		info))
	{
		case ERR_SUCCESS:
			break;
		case ERR_TASK_LISTS_FULL:
			pDlg->MessageBox(_T("您当前未完成的任务已达到最大线程数的16倍，请先完成当前任务。"),
				_T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
			GNT_END
		case ERR_CONNECTION_FAILED:
			pDlg->MessageBox(_T("无法连接到服务器。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			GNT_END
		case ERR_NO_NEW_TASK:
			pDlg->MessageBox(_T("暂时没有可用的新任务。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
			GNT_END
		default:
			pDlg->MessageBox(_T("下载新任务失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			GNT_END
	}

	pDlg->SendMessage(WM_THREADOPEN);
	pDlg->GetDlgItem(IDC_GET_NEW_TASK)->SetWindowText(_T("下载新任务(&N)"));
	CString strMsg;
	strMsg.Format(_T("Task #%06llu_L%u 成功添加至任务队列。\r\n请在 %s 前上传计算结果，否则任务将自动失效。"),
		info.id, info.layer, CTime(info.deadline).Format(_T("%c")));
	pDlg->MessageBox(strMsg, _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_GET_NEW_TASK);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTOUPLOAD);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTODOWNLOAD);

	return 0;
}

void CRamseyXDlg::OnBnClickedGetNewTask()
{
	AfxBeginThread(GetNewTaskProc, nullptr);
}

LRESULT CRamseyXDlg::OnUpdateStatus(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());
	return 0;
}

LRESULT CRamseyXDlg::OnEnableCtrl(WPARAM wParam, LPARAM lParam)
{
	CWnd *pCtrl = GetDlgItem(lParam);
	if (pCtrl)
		pCtrl->EnableWindow(wParam);
	return 0;
}

LRESULT CRamseyXDlg::OnClearMyList(WPARAM wParam, LPARAM lParam)
{
	CListCtrl *pList = static_cast<CListCtrl*>(GetDlgItem(IDC_ME));
	if (pList)
		pList->DeleteAllItems();
	return 0;
}

LRESULT CRamseyXDlg::OnAutoUpload(WPARAM wParam, LPARAM lParam)
{
	bool bUp = !IsDlgButtonChecked(IDC_DISABLE_AUTOUPLOAD);
	bool bDown = !IsDlgButtonChecked(IDC_DISABLE_AUTODOWNLOAD);

	if (m_bIsAutoUploading || (!bUp && !bDown))
		return 0;

	m_bIsAutoUploading = true;

	unsigned int nParam = 0;

	nParam |= (bUp ? 1U : 0U);  // true - upload
	nParam |= (bDown ? 1U : 0U) << 1;  // true - download

	AfxBeginThread(AutoUploadProc, reinterpret_cast<LPVOID>(nParam));

	return 0;
}

LRESULT CRamseyXDlg::OnWriteLog(WPARAM wParam, LPARAM lParam)
{
	m_controller.writeLog();

	return 0;
}

LRESULT CRamseyXDlg::OnClrOutdated(WPARAM wParam, LPARAM lParam)
{
	m_controller.clearOutdated();

	return 0;
}

UINT UpdateBenchmarkProc(LPVOID lpParam)
{
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

	TCHAR lszComputerName[MAX_PATH + 2];
	DWORD dwSize = MAX_PATH;
	::GetComputerName(lszComputerName, &dwSize);

	pDlg->m_controller.updateBenchmark(
		LPCTSTR(pDlg->m_strUsername),
		LPCTSTR(pDlg->m_strPassword),
		lszComputerName,
		RamseyXUtils::getCPUBrandString(),
		pDlg->m_fVAX_MIPS);

	return 0;
}