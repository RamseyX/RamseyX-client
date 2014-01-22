
// RamseyXDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "RamseyX.h"
#include "RamseyXDlg.h"
#include "afxdialogex.h"
#include "CPUInfo.h"
#include "LoginDlg.h"
#include "Splash.h"
extern "C" 
{
	#include "dhry.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT ThreadProc(LPVOID lpParam)
{
	CRamseyXDlg* pDlg = static_cast<CRamseyXDlg*>(AfxGetApp()->GetMainWnd());
	int nID = reinterpret_cast<int>(lpParam);
	RamseyXTask *pTask = new RamseyXTask;

	csRunning.Lock();
	POSITION pos = pDlg->m_runningTaskQueue.GetHeadPosition();
	while (pos && pDlg->m_runningTaskQueue.GetAt(pos).threadID != nID)
		pDlg->m_runningTaskQueue.GetNext(pos);
	if (!pos)
	{
		csRunning.Unlock();
		pDlg->MessageBox(_T("错误的线程设置。任务未运行。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		return 0;
	}
	RXTASKINFO info = pDlg->m_runningTaskQueue.GetAt(pos);
	csRunning.Unlock();
	
	pDlg->m_aTasks[nID] = pTask;

	pTask->launch(info);
	
	info.offset = info.blockLength;
	csRunning.Lock();
	pDlg->m_runningTaskQueue.RemoveAt(pos);
	csRunning.Unlock();
	csCompleted.Lock();
	pDlg->m_completedTaskQueue.AddTail(info);
	csCompleted.Unlock();

	delete pDlg->m_aTasks[nID];
	pDlg->m_aTasks[nID] = NULL;
	pDlg->m_aWorkingThreads[nID] = NULL;

	csBatch.Lock();
	pos = pDlg->m_batchQueue.GetHeadPosition();
	while (pos && pDlg->m_batchQueue.GetAt(pos).localCounter != info.batch)
		pDlg->m_batchQueue.GetNext(pos);
	if (pos)
	{
		RXBATCHINFO batch = pDlg->m_batchQueue.GetAt(pos);
		for (int i = 0; i < batch.numOfTasks; i++)
			if (batch.taskID[i] == info.id)
			{
				batch.taskCompleted[i] = true;
				break;
			}
		pDlg->m_batchQueue.SetAt(pos, batch);
	}
	csBatch.Unlock();

	pDlg->SendMessage(WM_THREADOPEN);

	return 0;
}

UINT FetchProc(LPVOID lpParam)
{
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

	pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_FETCH_NOW);
	pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("获取中..."));

	MyCurlWrapper conn;
	
	// What's up
	if (!conn.StandardOpt("www.ramseyx.org/whats_up.php") || !conn.Execute())
	{
		pDlg->GetDlgItem(IDC_WHATSUP)->SetWindowText(_T("获取动态不成功。"));
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_FETCH_NOW);
		pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("立即获取(&F)"));

		return 0;
	}
	else
	{
		if (conn.GetErrorCode() != ERR_SUCCESS)
		{
			pDlg->GetDlgItem(IDC_WHATSUP)->SetWindowText(_T("获取动态不成功。"));
			pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_FETCH_NOW);
			pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("立即获取(&F)"));

			return 0;
		}
		pDlg->GetDlgItem(IDC_WHATSUP)->SetWindowText(conn.GetString());
	}

	// Get version
	if (!conn.StandardOpt("www.ramseyx.org/get_version.php") || !conn.Execute())
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_FETCH_NOW);
		pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("立即获取(&F)"));

		return 0;
	}
	else
	{
		if (conn.GetErrorCode() != ERR_SUCCESS)
		{
			pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_FETCH_NOW);
			pDlg->GetDlgItem(IDC_FETCH_NOW)->SetWindowText(_T("立即获取(&F)"));

			return 0;
		}
		int P = 0, S = 0, T = 0;
		swscanf_s(conn.GetString(), _T("%d %d %d"), &P, &S, &T);
		if (PRIMARY_VERSION < P || (PRIMARY_VERSION == P && (SECONDARY_VERSION < S || (SECONDARY_VERSION == S && TERTIARY_VERSION < T))))
		{
			CString strVersion;
			strVersion.Format(_T("%d.%d.%d"), P, S, T);

			TCHAR lszFilename[MAX_PATH + 2] = _T(".\\RamseyXUpdate.exe");
			SHELLEXECUTEINFO sei = {0};
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

	CListCtrl* pList = static_cast<CListCtrl*>(pDlg->GetDlgItem(IDC_RANK));
	// 清空旧列表
	pList->DeleteAllItems();

	pList = static_cast<CListCtrl*>(pDlg->GetDlgItem(IDC_ME));
	// 清空旧列表
	pList->DeleteAllItems();

	pList = static_cast<CListCtrl*>(pDlg->GetDlgItem(IDC_TOTAL));
	// 清空旧列表
	pList->DeleteAllItems();

	MyCurlWrapper conn;

	// 获取新资料
	if (!conn.StandardOpt("www.ramseyx.org/get_project_info.php") || !conn.Execute() || conn.GetErrorCode() != ERR_SUCCESS)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}
	
	CString strProgress, strTime, strNumOfTasks, strPower;
	TCHAR lszNumOfUsers[MAX_PATH] = {0}, lszNumOfMachines[MAX_PATH] = {0};
	unsigned __int64 nTime = 0, nNumOfTasksFinished = 0, nNumOfTasks = 0;
	float fCurrentPower = 0.0f, fMaxPower = 0.0f;
	
	swscanf_s(conn.GetString(), _T("%llu %s %s %llu %llu %f %f"), &nNumOfTasksFinished, lszNumOfUsers, sizeof (lszNumOfUsers),
		lszNumOfMachines, sizeof (lszNumOfMachines), &nNumOfTasks, &nTime, &fCurrentPower, &fMaxPower);
	
	double fProgress = nNumOfTasksFinished * 100.0 / nNumOfTasks;
	if (100.0 - fProgress <= 0.01 && nNumOfTasksFinished < nNumOfTasks)
		fProgress = 99.99;
	strProgress.Format(_T("%.2f%%"), fProgress);
	strTime.Format(_T("%.2f"), nTime / 86400.0);
	strNumOfTasks.Format(_T("%llu"), nNumOfTasks);
	strPower.Format(_T("%.1f/%.1f"), fCurrentPower / 1000.0f, fMaxPower / 1000.0f);

	// 完成度 总用户数 总计算机数 总任务数 总时长(日) 当前/峰值算力(KDMIPS)
	pList->InsertItem(0, strProgress);
	pList->SetItem(0, 1, LVIF_TEXT, lszNumOfUsers, 0, 0, 0, 0, 0);
	pList->SetItem(0, 2, LVIF_TEXT, lszNumOfMachines, 0, 0, 0, 0, 0);
	pList->SetItem(0, 3, LVIF_TEXT, strNumOfTasks, 0, 0, 0, 0, 0);
	pList->SetItem(0, 4, LVIF_TEXT, strTime, 0, 0, 0, 0, 0);
	pList->SetItem(0, 5, LVIF_TEXT, strPower, 0, 0, 0, 0, 0);

	pList = static_cast<CListCtrl*>(pDlg->GetDlgItem(IDC_RANK));
	
	if (!conn.StandardOpt("www.ramseyx.org/get_top_20.php") || !conn.Execute() || conn.GetErrorCode() != ERR_SUCCESS)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}
	
	// 排行 用户名 积分 完成任务数 总时长(日)
	CString strRankData(conn.GetString());
	for (int i = 0; i < 20; i++)
	{
		CString strTmp;
		TCHAR lszBuffer[MAX_PATH] = {0};

		// rank
		strTmp.Format(_T("%d"), i + 1);
		pList->InsertItem(i, strTmp);
		
		// username
		swscanf_s(strRankData, _T("%s"), lszBuffer, sizeof (lszBuffer));
		strRankData = strRankData.Right(strRankData.GetLength() - strRankData.Find(_T(' '), 0) - 1);
		pList->SetItem(i, 1, LVIF_TEXT,lszBuffer, 0, 0, 0, 0, 0);

		// score
		swscanf_s(strRankData, _T("%s"), lszBuffer, sizeof (lszBuffer));
		strTmp.Format(_T("%.2f"), _wtof(lszBuffer));
		strRankData = strRankData.Right(strRankData.GetLength() - strRankData.Find(_T(' '), 0) - 1);
		pList->SetItem(i, 2, LVIF_TEXT, strTmp, 0, 0, 0, 0, 0);

		// number of tasks finished
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

	pList = static_cast<CListCtrl*>(pDlg->GetDlgItem(IDC_ME));

	if (!conn.StandardOpt("www.ramseyx.org/get_user_info.php") || !conn.SetPost())
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}
	conn.AddPostField("username", CStringA(pDlg->m_strUsername));
	conn.AddPostField("password", CStringA(pDlg->m_strPassword));
	if (!conn.Execute() || conn.GetErrorCode() != ERR_SUCCESS)
	{
		pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
		pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

		return 0;
	}
	
	CString strScore;
	nTime = nNumOfTasksFinished = 0;
	TCHAR lszRank[MAX_PATH] = {0}, lszScore[MAX_PATH] = {0}, lszNumOfRcmd[MAX_PATH] = {0}, lszRecommender[MAX_PATH] = {0};
	fCurrentPower = 0.0f;

	swscanf_s(conn.GetString(), _T("%s %s %llu %llu %s %s %f"), lszRank, sizeof (lszRank), lszScore, sizeof (lszScore),
		&nNumOfTasksFinished, &nTime,
		lszNumOfRcmd, sizeof (lszNumOfRcmd), lszRecommender, sizeof (lszRecommender), &fCurrentPower);
	
	strScore.Format(_T("%.2f"), _wtof(lszScore));
	strTime.Format(_T("%.2f"), nTime / 86400.0);
	strNumOfTasks.Format(_T("%llu"), nNumOfTasksFinished);
	strPower.Format(_T("%.1f"), fCurrentPower);

	// 排行 积分 总任务数 总时长(日) 推荐数 当前在线算力(DMIPS)
	pList->InsertItem(0, lszRank);
	pList->SetItem(0, 1, LVIF_TEXT, strScore, 0, 0, 0, 0, 0);
	pList->SetItem(0, 2, LVIF_TEXT, strNumOfTasks, 0, 0, 0, 0, 0);
	pList->SetItem(0, 3, LVIF_TEXT, strTime, 0, 0, 0, 0, 0);
	pList->SetItem(0, 4, LVIF_TEXT, lszNumOfRcmd, 0, 0, 0, 0, 0);
	pList->SetItem(0, 5, LVIF_TEXT, strPower, 0, 0, 0, 0, 0);

	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_REFRESH_RANK);
	pDlg->GetDlgItem(IDC_REFRESH_RANK)->SetWindowText(_T("刷新(&R)"));

	return 0;
}

UINT AutoUploadProc(LPVOID lpParam)
{
	CRamseyXDlg* pDlg = static_cast<CRamseyXDlg*>(AfxGetApp()->GetMainWnd());
	int nParam = reinterpret_cast<int>(lpParam);
	bool bUpload = (nParam & 1) != 0;
	bool bDownload = (nParam & (1 << 1)) != 0;

	CStringA strUsername(pDlg->m_strUsername), strPassword(pDlg->m_strPassword);

	MyCurlWrapper conn;

	// 版本检查
	if (!conn.StandardOpt("www.ramseyx.org/get_version.php") || !conn.Execute())
	{
		pDlg->m_bIsAutoUploading = false;
		return 0;
	}
	if (conn.GetErrorCode() != 0)
	{
		pDlg->m_bIsAutoUploading = false;
		return 0;
	}
	int P = 0, S = 0, T = 0;
	swscanf_s(conn.GetString(), _T("%d %d %d"), &P, &S, &T);
	if (PRIMARY_VERSION < P || (PRIMARY_VERSION == P && SECONDARY_VERSION < S))
	{
		CString strVersion;
		strVersion.Format(_T("%d.%d.%d"), P, S, T);

		if (IDOK == pDlg->MessageBox(_T("您的客户端版本过低，无法上传或下载，请更新至最新版程序。本地日志将会被保留。点击确定立刻更新至 ") + strVersion + _T("。"), _T("RamseyX 运算客户端"), MB_OKCANCEL | MB_ICONINFORMATION))
		{
			TCHAR lszFilename[MAX_PATH + 2] = _T(".\\RamseyXUpdate.exe");
			SHELLEXECUTEINFO sei = {0};
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

			pDlg->m_bIsAutoUploading = false;
			return 0;
		}
		pDlg->m_bIsAutoUploading = false;
		return 0;
	}
	
	// 上传
	if (bUpload)
	{
		char szComputerName[MAX_PATH + 2];
		DWORD dwSize = MAX_PATH;
		::GetComputerNameA(szComputerName, &dwSize);
		CStringA strCPUBrand(CCPUInfo().GetBrand());
		
		csCompleted.Lock();
		int nUpNum = pDlg->m_completedTaskQueue.GetCount();
		while (nUpNum-- > 0)
		{
			RXTASKINFO info = pDlg->m_completedTaskQueue.GetHead();
			csCompleted.Unlock();
			if (!conn.StandardOpt("www.ramseyx.org/upload.php") || !conn.SetPost())
			{
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}

			CStringA strTmp;
			conn.AddPostField("username", strUsername);				// username
			conn.AddPostField("password", strPassword);				// password
			strTmp.Format("%llu", info.id);
			conn.AddPostField("task_id", strTmp);					// task_id
			strTmp.Format("%d", info.result);
			conn.AddPostField("task_result", strTmp);				// task_result
			strTmp.Format("%llu", pDlg->m_tTime);
			conn.AddPostField("time", strTmp);						// time
			strTmp.Format("%llu", info.complexity / 3);
			conn.AddPostField("block_length", strTmp);				// block_length
			conn.AddPostField("computer_name", szComputerName);		// computer_name
			conn.AddPostField("cpu_brand", strCPUBrand);			// cpu_brand

			if (!conn.Execute())
			{
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}
			if (conn.GetErrorCode() != ERR_SUCCESS)
			{
				if (conn.GetErrorCode() == ERR_TASK_OUTDATED)
				{
					csCompleted.Lock();
					pDlg->m_completedTaskQueue.RemoveHead();
					csCompleted.Unlock();
					pDlg->SendMessage(WM_WRITELOG);
					continue;
				}
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}
			csCompleted.Lock();
			pDlg->m_completedTaskQueue.RemoveHead();
			csCompleted.Unlock();
			pDlg->m_tTime = 0;
			pDlg->SendMessage(WM_WRITELOG);
			csCompleted.Lock();
		}
		csCompleted.Unlock();
		pDlg->SendMessage(WM_THREADOPEN);
		pDlg->OnBnClickedRefreshRank();
	}


	// get new task
	if (bDownload)
	{
		RamseyXTask task;
		csRunning.Lock();
		csTodo.Lock();
		int nDownNum = pDlg->m_nThreadNum * 8 - pDlg->m_runningTaskQueue.GetCount() - pDlg->m_todoTaskQueue.GetCount();
		csRunning.Unlock();
		csTodo.Unlock();
		while (nDownNum-- > 0)
		{
			if (!conn.StandardOpt("www.ramseyx.org/get_new_task.php") || !conn.SetPost())
			{
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}
		
			CStringA strPVer, strSVer;
			strPVer.Format("%d", PRIMARY_VERSION);
			strSVer.Format("%d", SECONDARY_VERSION);
		
			conn.AddPostField("p_ver", strPVer);
			conn.AddPostField("s_ver", strSVer);
			conn.AddPostField("username", strUsername);
			conn.AddPostField("password", strPassword);
				
			if (!conn.Execute())
			{
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}
			if (conn.GetErrorCode() != ERR_SUCCESS)
			{
				if (conn.GetErrorCode() == ERR_NO_NEW_TASK)
					break;
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}
		
			RXTASKINFO *pNewTask = new RXTASKINFO;
			TCHAR lszVerificationCode[100] = {0};
			swscanf_s(conn.GetString(), _T("%llu %llu %llu %s"), &pNewTask->id, &pNewTask->combinationNum, &pNewTask->block, lszVerificationCode, sizeof (lszVerificationCode));
			// task_id combination_num block verification_code
		
			if (!conn.StandardOpt("www.ramseyx.org/confirm_task.php") || !conn.SetPost())
			{
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}
			
			CStringA strTID;
			strTID.Format("%llu", pNewTask->id);
			conn.AddPostField("username", strUsername);
			conn.AddPostField("task_id", strTID);
			conn.AddPostField("verification_code", CStringA(lszVerificationCode));

			if (!conn.Execute())
			{
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}
			if (conn.GetErrorCode() != ERR_SUCCESS)
			{
				pDlg->m_bIsAutoUploading = false;
				return 0;
			}

			pNewTask->offset = 0;
			pDlg->SendMessage(WM_NEWTASK, 0, reinterpret_cast<LPARAM>(pNewTask));
			pDlg->SendMessage(WM_WRITELOG);
		}
		pDlg->SendMessage(WM_THREADOPEN);
	}

	pDlg->SendMessage(WM_UPDATESTATUS);

	if (!pDlg->m_bIsRunning)// && pDlg->m_bWasRunning)
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
	  m_tTime(0),
	  m_tLastLog(0),
	  m_tLastBenchmark(0),
	  m_nCPULimit(0),
	  m_bIsRunning(false),
	  m_bWasRunning(false),
	  m_bIsAuto(false),
	  m_bIsUpdating(false),
	  m_bIsLocked(false),
	  m_bIsExiting(false),
	  m_bIsAutoUploading(false),
	  m_hTimerQueue(NULL),
	  m_aWorkingThreads(NULL),
	  m_aTasks(NULL),
	  m_fVAX_MIPS(0.0f),
	  m_fTotalVAX_MIPS(0.0f)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	memset(m_aTimerHandles, NULL, sizeof (m_aTimerHandles));
}

void CRamseyXDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

CString CRamseyXDlg::GetStatus()
{
	CString str(_T("最后保存: ")
		+ CTime(m_tLastLog).Format(_T("%c"))
		+ _T("\r\n"));
	str.Format(_T("%s单线程算力：%.1f DMIPS\r\n"), str, m_fVAX_MIPS);
	if (m_bIsRunning)
	{
		int nPos = m_tTime % 20;
		str += _T("状态：运算中 [");
		for (int i = 0; i < nPos; i++)
			str += _T(' ');
		str += _T('●');
		for (int i = nPos + 1; i < 20; i++)
			str += _T(' ');
		str += _T("]\r\n");
	}
	else
		str += _T("状态：暂停中 [●                   ]\r\n");
	str.Format(_T("%s运算时长：%llu秒\r\n-------------------------------------\r\n"), str, m_tTime);
	
	csBatch.Lock();
	POSITION pos = m_batchQueue.GetHeadPosition();
	while (pos)
	{
		RXBATCHINFO batch = m_batchQueue.GetAt(pos);
		int nCompleted = 0;
		for (int i = 0; i < batch.numOfTasks; i++)
			if (batch.taskCompleted[i])
				nCompleted++;
		str.Format(_T("%sBatch_%llu_%d_%d_%d: %.2f%%\r\n"), str, m_uID, batch.identifier, batch.numOfTasks, batch.clockID, nCompleted * 100.0 / batch.numOfTasks);
		m_batchQueue.GetNext(pos);
	}
	csBatch.Unlock();
	
	/*// 进行中任务
	csRunning.Lock();
	//str.Format(_T("%s进行中任务：%d\r\n"), str, m_runningTaskQueue.GetCount());
	pos = m_runningTaskQueue.GetHeadPosition();
	while (pos)
	{
		RXTASKINFO info = m_runningTaskQueue.GetAt(pos);
		if (m_aTasks[info.threadID] && m_aTasks[info.threadID]->located)
		{
			info.offset = m_aTasks[info.threadID]->progress - m_aTasks[info.threadID]->blockBegin;
			info.complexity += m_aTasks[info.threadID]->complexity;
			m_aTasks[info.threadID]->complexity = 0;
			m_runningTaskQueue.SetAt(pos, info);
		}
		m_runningTaskQueue.GetNext(pos);
		//str.Format(_T("%sTask #%06llu：%.2f%%\r\n"), str, info.id, info.offset * 100.0 / info.blockLength);
	}
	csRunning.Unlock();

	/*csTodo.Lock();
	// 等待中任务
	str.Format(_T("%s-------------------------------------\r\n等待中任务：%d\r\n"), str, m_todoTaskQueue.GetCount());
	pos = m_todoTaskQueue.GetHeadPosition();
	for (int i = 0; i < m_todoTaskQueue.GetCount() && i < 3; i++)
	{
		RXTASKINFO info = m_todoTaskQueue.GetNext(pos);
		str.Format(_T("%sTask #%06llu：%.2f%%\r\n"), str, info.id, info.offset * 100.0 / info.blockLength);
	}
	if (m_todoTaskQueue.GetCount() > 3)
		str.Format(_T("%s(%d more...)\r\n"), str, m_todoTaskQueue.GetCount() - 3);
	csTodo.Unlock();

	csCompleted.Lock();
	
	// 已完成任务
	str.Format(_T("%s-------------------------------------\r\n已完成任务：%d\r\n"), str, m_completedTaskQueue.GetCount());
	pos = m_completedTaskQueue.GetHeadPosition();
	for (int i = 0; i < m_completedTaskQueue.GetCount() && i < 3; i++)
	{
		RXTASKINFO info = m_completedTaskQueue.GetNext(pos);
		str.Format(_T("%sTask #%06llu：%.2f%%\r\n"), str, info.id, info.offset * 100.0 / info.blockLength);
	}
	if (m_completedTaskQueue.GetCount() > 3)
		str.Format(_T("%s(%d more...)\r\n"), str, m_completedTaskQueue.GetCount() - 3);
	csCompleted.Unlock();*/

	return str;
}

BOOL CRamseyXDlg::WriteLog()
{
	// 文件格式
	/*
	time_t m_tLastLog
	time_t m_tTime
	int    m_runningTaskQueue.GetCount()
	RXTASKINFOs
	int    m_todoTaskQueue.GetCount()
	RXTASKINFOs
	int    m_completedTaskQueue.GetCount()
	RXTASKINFOs
	*/
	csWriteLog.Lock();
	
	CFile file;
	if (!file.Open(m_strDir + _T("RamseyX.data5"), CFile::modeCreate | CFile::modeWrite))
	{
		csWriteLog.Unlock();
		return FALSE;
	}

	m_tLastLog = CTime::GetCurrentTime().GetTime();
	file.Write(&m_tLastLog, sizeof (time_t));
	file.Write(&m_tTime, sizeof (time_t));
	
	csRunning.Lock();
	int nSize = m_runningTaskQueue.GetCount();
	file.Write(&nSize, sizeof (int));
	POSITION pos = m_runningTaskQueue.GetHeadPosition();
	while (pos)
		file.Write(&m_runningTaskQueue.GetNext(pos),  sizeof (RXTASKINFO));
	csRunning.Unlock();

	csTodo.Lock();
	nSize = m_todoTaskQueue.GetCount();
	file.Write(&nSize, sizeof (int));
	pos = m_todoTaskQueue.GetHeadPosition();
	while (pos)
		file.Write(&m_todoTaskQueue.GetNext(pos),  sizeof (RXTASKINFO));
	csTodo.Unlock();
	
	csCompleted.Lock();
	nSize = m_completedTaskQueue.GetCount();
	file.Write(&nSize, sizeof (int));
	pos = m_completedTaskQueue.GetHeadPosition();
	while (pos)
		file.Write(&m_completedTaskQueue.GetNext(pos),  sizeof (RXTASKINFO));
	csCompleted.Unlock();

	file.Close();

	csWriteLog.Unlock();

	return TRUE;
}

BOOL CRamseyXDlg::ReadLog()
{
	CFile file;

	// 文件不存在
	if (!file.Open(m_strDir + _T("RamseyX.data5"), CFile::modeRead))
		return FALSE;

	// 文件格式错误
	if (sizeof (time_t) != file.Read(&m_tLastLog, sizeof (time_t)))
		return FALSE;
	if (sizeof (time_t) != file.Read(&m_tTime, sizeof (time_t)))
		return FALSE;
	
	int nSize;
	RXTASKINFO info;

	if (sizeof (int) != file.Read(&nSize, sizeof (int)))
		return FALSE;
	for (int i = 0; i < nSize; i++)
	{
		if (sizeof (RXTASKINFO) != file.Read(&info, sizeof (RXTASKINFO)))
			return FALSE;
		if (i < m_nThreadNum)
		{
			info.threadID = i;
			csRunning.Lock();
			m_runningTaskQueue.AddTail(info);
			csRunning.Unlock();
			m_aWorkingThreads[i] = AfxBeginThread(ThreadProc, reinterpret_cast<LPVOID>(i), 0, 0, CREATE_SUSPENDED);
		}
		else
		{
			csTodo.Lock();
			m_todoTaskQueue.AddTail(info);
			csTodo.Unlock();
		}
	}

	if (sizeof (int) != file.Read(&nSize, sizeof (int)))
		return FALSE;
	for (int i = 0; i < nSize; i++)
	{
		if (sizeof (RXTASKINFO) != file.Read(&info, sizeof (RXTASKINFO)))
			return FALSE;
		csTodo.Lock();
		m_todoTaskQueue.AddTail(info);
		csTodo.Unlock();
	}

	if (sizeof (int) != file.Read(&nSize, sizeof (int)))
		return FALSE;
	for (int i = 0; i < nSize; i++)
	{
		if (sizeof (RXTASKINFO) != file.Read(&info, sizeof (RXTASKINFO)))
			return FALSE;
		csCompleted.Lock();
		m_completedTaskQueue.AddTail(info);
		csCompleted.Unlock();
	}

	return TRUE;
}

void CRamseyXDlg::AddNewTask(RXTASKINFO &info)
{
	info.complexity = info.offset = 0;
	info.deadline = time(NULL) + MAX_DAYS * 24 * 3600;

	int i;
	for (i = 0; i < m_nThreadNum; i++)
		if (m_aWorkingThreads[i] == NULL)
			break;
	if (i < m_nThreadNum)
	{
		// add to m_runningTaskQueue
		info.threadID = i;
		csRunning.Lock();
		m_runningTaskQueue.AddTail(info);
		csRunning.Unlock();
		if (m_bIsRunning)
			m_aWorkingThreads[i] = AfxBeginThread(ThreadProc, reinterpret_cast<LPVOID>(i));
		else
			m_aWorkingThreads[i] = AfxBeginThread(ThreadProc, reinterpret_cast<LPVOID>(i), 0, 0, CREATE_SUSPENDED);
	}
	else
	{
		csTodo.Lock();
		m_todoTaskQueue.AddTail(info); // add to m_todoTaskQueue
		csTodo.Unlock();
	}
}

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	CRamseyXDlg* pDlg = static_cast<CRamseyXDlg*>(AfxGetApp()->GetMainWnd());
	switch (reinterpret_cast<UINT>(lpParam))
	{
	case 1:	// Update
		pDlg->m_tTime++;
		pDlg->m_fTotalVAX_MIPS += pDlg->m_fVAX_MIPS * pDlg->m_nThreadNum;
		pDlg->SendMessage(WM_UPDATESTATUS);
		break;
	case 2:	// Log
		pDlg->SendMessage(WM_WRITELOG);
		break;
	case 3:	// Fetch whatsup & Check for update & Update benchmark
		pDlg->SendMessage(WM_CLROUTDATED);
		AfxBeginThread(FetchProc, NULL);
		if (pDlg->m_bIsLocked)
			AfxBeginThread(UpdateBenchmarkProc, NULL);
		break;
	case 4:	// Auto upload
		pDlg->SendMessage(WM_AUTOUPLOAD);
		break;
	default:
		break;
	}
}

// 1 - succeeded
// NULL - failed
UINT_PTR CRamseyXDlg::SetTimer(UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK* lpfnTimer)(HWND, UINT, UINT_PTR, DWORD))
{
	if (nIDEvent <= 0 || nIDEvent >= 10)
		return NULL;
	if (!KillTimer(nIDEvent))
		return NULL;

	if (!::CreateTimerQueueTimer(&(m_aTimerHandles[nIDEvent]), m_hTimerQueue, 
            (WAITORTIMERCALLBACK)TimerRoutine, reinterpret_cast<PVOID>(nIDEvent), nElapse, nElapse, WT_EXECUTEDEFAULT))
	{
		m_aTimerHandles[nIDEvent] = NULL;
		return NULL;
	}
	return 1;
}

BOOL CRamseyXDlg::KillTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent <= 0 || nIDEvent >= 10)
		return FALSE;
	if (!m_aTimerHandles[nIDEvent])
		return TRUE;

	if (::DeleteTimerQueueTimer(m_hTimerQueue, m_aTimerHandles[nIDEvent], NULL))
	{
		m_aTimerHandles[nIDEvent] = NULL;
		return TRUE;
	}
	else
		return FALSE;
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
	ON_MESSAGE(WM_NEWTASK, &CRamseyXDlg::OnNewTask)
	ON_MESSAGE(WM_NEWBATCH, &CRamseyXDlg::OnNewBatch)
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
	::curl_global_init(CURL_GLOBAL_DEFAULT);

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
	::GetModuleFileName(NULL, lszBuffer, MAX_PATH);
	for (int i = lstrlen(lszBuffer) - 1; i >= 0 && lszBuffer[i] != _T('\\'); i--)
		lszBuffer[i] = _T('\0');
	::SetCurrentDirectory(lszBuffer);

	// 获取开机运行状态
	CRegKey reg;
	::GetModuleFileName(NULL, lszBuffer, MAX_PATH);
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
	::SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, lszBuffer);
	m_strDir = lszBuffer + CString(_T("\\RamseyX\\"));
	::CreateDirectory(m_strDir, NULL);

	// 获取CPU信息
	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	m_nCoreNum = si.dwNumberOfProcessors;
	m_nThreadNum = m_nCoreNum;
	m_aWorkingThreads = new CWinThread *[m_nCoreNum];
	m_aTasks = new RamseyXTask *[m_nCoreNum];
	memset(m_aWorkingThreads, NULL, sizeof (CWinThread *) * m_nCoreNum);
	memset(m_aTasks, NULL, sizeof (RamseyXTask *) * m_nCoreNum);

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
	m_nCPULimit = 100; // TODO

	// 载入本地日志
	::DeleteFile(m_strDir + _T("RamseyX.data4"));
	if (!ReadLog())	// 不存在或格式错误
	{
		m_tTime = 0;
		m_runningTaskQueue.RemoveAll();
		m_todoTaskQueue.RemoveAll();
		m_completedTaskQueue.RemoveAll();
		WriteLog();
	}
	
	// 载入账户设置
	if (file.Open(m_strDir + _T("RamseyX.ac"), CFile::modeRead))
	{
		TCHAR lszBuf[100];
		int nLen;
		
		VERIFY(sizeof (int) == file.Read(&nLen, sizeof (int)));
		VERIFY(sizeof (TCHAR) * nLen == file.Read(lszBuf, sizeof (TCHAR) * nLen));
		m_strUsername = lszBuf;

		VERIFY(sizeof (int) == file.Read(&nLen, sizeof (int)));
		VERIFY(sizeof (TCHAR) * nLen == file.Read(lszBuf, sizeof (TCHAR) * nLen));
		m_strPassword = lszBuf;

		VERIFY(sizeof (unsigned long long) == file.Read(&m_uID, sizeof (unsigned long long)));
		file.Close();

		SetTimer(4, 60000, NULL);
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
	pList->InsertColumn(2, _T("积分"), LVCFMT_RIGHT, 70);
	pList->InsertColumn(3, _T("完成任务数"), LVCFMT_RIGHT, 80);
	pList->InsertColumn(4, _T("总时长(日)"), LVCFMT_RIGHT, 75);
	pList->InsertColumn(5, _T("当前在线算力(DMIPS)"), LVCFMT_RIGHT, 145);

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
	m_fVAX_MIPS = 5000.0f; //::Benchmark();
	//RamseyXTask task;
	//task.printSQLScript(0, 0);
	if (!m_bIsAuto)
		splash.SendMessage(WM_BENCHMARK, TRUE);

	if (!m_bIsLocked)
		pList->EnableWindow(FALSE);

	// 获取动态
	SetTimer(3, 600000, NULL);
	
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
	
	// 刷新状态
	GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());

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

	NOTIFYICONDATA nid;
	nid.cbSize = sizeof (NOTIFYICONDATA);
	nid.hWnd = m_hWnd;
	nid.uID = IDR_MAINFRAME;
	::Shell_NotifyIcon(NIM_DELETE, &nid);

	::DeleteTimerQueue(m_hTimerQueue);

	for (int i = 0; i < m_nThreadNum; i++)
		if (m_aWorkingThreads[i])
		{
			::TerminateThread(m_aWorkingThreads[i]->m_hThread, 0);
			if (m_aTasks[i])
				delete m_aTasks[i];
		}
	delete [] m_aWorkingThreads;
	delete [] m_aTasks;

	CDialogEx::OnClose();
}


void CRamseyXDlg::OnBnClickedStart()
{
	if (m_bIsRunning)
	{
		for (int i = 0; i < m_nThreadNum; i++)
			if (m_aWorkingThreads[i])
				m_aWorkingThreads[i]->SuspendThread();

		KillTimer(1);
		KillTimer(2);
		m_bIsRunning = m_bWasRunning = false;
		GetDlgItem(IDC_START)->SetWindowText(_T("开始运算(&S)"));
		GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());
	}
	else
	{
		csRunning.Lock();
		if (m_runningTaskQueue.IsEmpty())
		{
			csRunning.Unlock();
			//MessageBox(_T("当前没有可进行的任务。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
			return;
		}
		csRunning.Unlock();
		
		for (int i = 0; i < m_nThreadNum; i++)
			if (m_aWorkingThreads[i])
				m_aWorkingThreads[i]->ResumeThread();
			
		SetTimer(1, 1000, NULL);
		SetTimer(2, 10000, NULL);
		m_tLastBenchmark = CTime::GetCurrentTime().GetTime();
		m_bIsRunning = true;
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
		if (m_bIsRunning)
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
	csTodo.Lock();
	csRunning.Lock();
	while (!m_todoTaskQueue.IsEmpty() && m_runningTaskQueue.GetCount() < m_nThreadNum)
	{
		RXTASKINFO info = m_todoTaskQueue.GetHead();
		int i;
		for (i = 0; i < m_nThreadNum; i++)
			if (m_aWorkingThreads[i] == NULL)
				break;
		if (i < m_nThreadNum)
		{
			info.threadID = i;
			m_runningTaskQueue.AddTail(info);
			m_todoTaskQueue.RemoveHead();
			if (m_bIsRunning)
				m_aWorkingThreads[i] = AfxBeginThread(ThreadProc, reinterpret_cast<LPVOID>(i));
			else
				m_aWorkingThreads[i] = AfxBeginThread(ThreadProc, reinterpret_cast<LPVOID>(i), 0, 0, CREATE_SUSPENDED);
		}
		else
			break;
	}

	if (m_runningTaskQueue.IsEmpty() && m_todoTaskQueue.IsEmpty() && m_bIsRunning)
	{
		m_bWasRunning = true;
		OnBnClickedStart();
		SendMessage(WM_AUTOUPLOAD);
	}
	csTodo.Unlock();
	csRunning.Unlock();

	//GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());
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
		::GetModuleFileName(NULL, lszBuffer, MAX_PATH);
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
			file.Write(lszBuffer, sizeof (TCHAR) * nLen);
			
			lstrcpy(lszBuffer, m_strPassword);
			nLen = lstrlen(lszBuffer) + 1;
			file.Write(&nLen, sizeof (int));
			file.Write(lszBuffer, sizeof (TCHAR) * nLen);

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
	AfxBeginThread(FetchProc, NULL);
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
	
	// 调整进行队列中任务数量
	
	// 目前未全开
	if (m_runningTaskQueue.GetCount() < m_nThreadNum && !m_todoTaskQueue.IsEmpty())
		SendMessage(WM_THREADOPEN);
	// 目前超过最大并发
	else
	{
		csRunning.Lock();
		csTodo.Lock();
		while (m_runningTaskQueue.GetCount() > m_nThreadNum)
		{
			RXTASKINFO info = m_runningTaskQueue.GetTail();
			::TerminateThread(m_aWorkingThreads[info.threadID]->m_hThread, 0);
			m_aWorkingThreads[info.threadID] = NULL;
			delete m_aTasks[info.threadID];
			m_aTasks[info.threadID] = NULL;
			m_runningTaskQueue.RemoveTail();
			m_todoTaskQueue.AddHead(info);
		}
		csTodo.Unlock();
		csRunning.Unlock();
	}

	WriteLog();
	StoreLevelSpeed();
	GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());

	*pResult = 0;
}


void CRamseyXDlg::OnBnClickedRefreshRank()
{
	AfxBeginThread(RefreshRankProc, NULL);
}

void CRamseyXDlg::OnNMClickSyslink(NMHDR *pNMHDR, LRESULT *pResult)
{
	::ShellExecute(NULL, _T("open"), _T("http://www.ramseyx.org"), NULL, NULL, SW_SHOW);
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

	MyCurlWrapper conn;
	// check for update
	if (!conn.StandardOpt(SVR_API("get_version.php")) || !conn.Execute())
	{
		pDlg->MessageBox(_T("无法连接到服务器。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		GNT_END
	}
	if (conn.GetErrorCode() != 0)
	{
		pDlg->MessageBox(_T("版本验证失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		GNT_END
	}
	int P = 0, S = 0, T = 0;
	swscanf_s(conn.GetString(), _T("%d %d %d"), &P, &S, &T);
	if (PRIMARY_VERSION < P || (PRIMARY_VERSION == P && SECONDARY_VERSION < S))
	{
		CString strVersion;
		strVersion.Format(_T("%d.%d.%d"), P, S, T);

		if (IDOK == pDlg->MessageBox(_T("您的客户端版本过低，无法下载新任务，请更新至最新版程序。点击确定立刻更新至 ") + strVersion + _T("。"), _T("RamseyX 运算客户端"), MB_OKCANCEL | MB_ICONINFORMATION))
		{
			TCHAR lszFilename[MAX_PATH + 2] = _T(".\\RamseyXUpdate.exe");
			SHELLEXECUTEINFO sei = {0};
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

	// get new task
	pDlg->GetDlgItem(IDC_GET_NEW_TASK)->SetWindowText(_T("下载中...0%"));
	if (!conn.StandardOpt(SVR_API("get_new_task.php")) || !conn.SetPost())
	{
		pDlg->MessageBox(_T("下载新任务失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		GNT_END
	}
	CStringA strPVer, strSVer;
	strPVer.Format("%d", PRIMARY_VERSION);
	strSVer.Format("%d", SECONDARY_VERSION);
	conn.AddPostField("username", CStringA(pDlg->m_strUsername));
	conn.AddPostField("password", CStringA(pDlg->m_strPassword));
	conn.AddPostField("p_ver", strPVer);
	conn.AddPostField("s_ver", strSVer);
	
	if (!conn.Execute())
	{
		pDlg->MessageBox(_T("无法连接到服务器。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		GNT_END
	}
	switch (conn.GetErrorCode())
	{
	case ERR_SUCCESS:
		break;
	case ERR_NO_NEW_TASK:
		pDlg->MessageBox(_T("暂时没有可用的新任务。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
		GNT_END
	default:
		pDlg->MessageBox(_T("下载新任务失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		GNT_END
	}

	CString strData(conn.GetString());
	int nIdentifier = _wtoi(strData);
	strData = strData.Right(strData.GetLength() - strData.Find(_T(' '), 0) - 1);
	int nTaskNum = _wtoi(strData);
	strData = strData.Right(strData.GetLength() - strData.Find(_T(' '), 0) - 1);

	RXBATCHINFO *pNewBatch = new RXBATCHINFO;
	pNewBatch->numOfTasks = nTaskNum;
	pNewBatch->identifier = nIdentifier;
	pNewBatch->localCounter = RXBATCHINFO::counter++;

	CString strTitle;
	RXTASKINFO *pNewTask = NULL;
	for (int i = 0; i < nTaskNum; i++)
	{
		pNewTask = new RXTASKINFO;

		swscanf_s(strData, _T("%llu %llu %llu"), &pNewTask->id, &pNewTask->combinationNum, &pNewTask->block);
		strData = strData.Right(strData.GetLength() - strData.Find(_T(' '), 0) - 1);
		strData = strData.Right(strData.GetLength() - strData.Find(_T(' '), 0) - 1);
		strData = strData.Right(strData.GetLength() - strData.Find(_T(' '), 0) - 1);
		
		pNewBatch->taskID[i] = pNewTask->id;
		pNewBatch->taskCompleted[i] = false;
		pNewTask->batch = pNewBatch->localCounter;
		pNewTask->offset = 0;

		pDlg->SendMessage(WM_NEWTASK, 0, reinterpret_cast<LPARAM>(pNewTask));

		strTitle.Format(_T("下载中...%d%%"), static_cast<int>(i * 100.0 / nTaskNum));
		pDlg->GetDlgItem(IDC_GET_NEW_TASK)->SetWindowText(strTitle);
	}

	int nClockID = pNewBatch->clockID = clock();
	pDlg->SendMessage(WM_NEWBATCH, 0, reinterpret_cast<LPARAM>(pNewBatch));
	pDlg->SendMessage(WM_THREADOPEN);
	pDlg->GetDlgItem(IDC_GET_NEW_TASK)->SetWindowText(_T("下载新任务(&N)"));
	CString strMsg;
	strMsg.Format(_T("Batch_%llu_%d_%d_%d 成功添加至任务队列。\r\n请在 %d 天内上传计算结果，否则任务将自动失效。"), pDlg->m_uID, nIdentifier, nTaskNum, nClockID, MAX_DAYS);
	pDlg->MessageBox(strMsg, _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_GET_NEW_TASK);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTOUPLOAD);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTODOWNLOAD);

	return 0;
}

void CRamseyXDlg::OnBnClickedGetNewTask()
{
	csBatch.Lock();
	if (m_batchQueue.GetCount() >= m_nCoreNum * 8)
	{
		csBatch.Unlock();
		MessageBox(_T("您当前等待中的任务已达到CPU核心数的8倍，请先完成当前任务。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		csBatch.Unlock();
		AfxBeginThread(GetNewTaskProc, NULL);
	}
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

 LRESULT CRamseyXDlg::OnNewTask(WPARAM wParam, LPARAM lParam)
 {
	 RXTASKINFO *pInfo = reinterpret_cast<RXTASKINFO *>(lParam);
	 if (pInfo)
	 {
		 AddNewTask(*pInfo);
		 delete pInfo;
	 }
	 return 0;
 }

 LRESULT CRamseyXDlg::OnNewBatch(WPARAM wParam, LPARAM lParam)
 {
	 RXBATCHINFO *pInfo = reinterpret_cast<RXBATCHINFO *>(lParam);
	 if (pInfo)
	 {
		 csBatch.Lock();
		 m_batchQueue.AddTail(*pInfo);
		 csBatch.Unlock();
		 WriteLog();
		 GetDlgItem(IDC_STATUS)->SetWindowText(GetStatus());
		 delete pInfo;
	 }
	 return 0;
 }

 LRESULT CRamseyXDlg::OnAutoUpload(WPARAM wParam, LPARAM lParam)
 {
	 bool bUp = !IsDlgButtonChecked(IDC_DISABLE_AUTOUPLOAD) && !m_completedTaskQueue.IsEmpty();
	 bool bDown = !IsDlgButtonChecked(IDC_DISABLE_AUTODOWNLOAD) && m_runningTaskQueue.GetCount() + m_todoTaskQueue.GetCount() < m_nThreadNum * 8;

	 if (m_bIsAutoUploading || (!bUp && !bDown))
		 return 0;
	 
	 m_bIsAutoUploading = true;

	 int nParam = 0;

	 nParam |= (bUp ? 1 : 0);  // true - upload
	 nParam |= (bDown ? 1 : 0) << 1;  // true - download
	 
	 AfxBeginThread(AutoUploadProc, reinterpret_cast<LPVOID>(nParam));
	 
	 return 0;
 }

LRESULT CRamseyXDlg::OnWriteLog(WPARAM wParam, LPARAM lParam)
{
	csRunning.Lock();
	//str.Format(_T("%s进行中任务：%d\r\n"), str, m_runningTaskQueue.GetCount());
	POSITION pos = m_runningTaskQueue.GetHeadPosition();
	while (pos)
	{
		RXTASKINFO info = m_runningTaskQueue.GetAt(pos);
		if (m_aTasks[info.threadID] && m_aTasks[info.threadID]->located)
		{
			info.offset = m_aTasks[info.threadID]->progress - m_aTasks[info.threadID]->blockBegin;
			info.complexity += m_aTasks[info.threadID]->complexity;
			m_aTasks[info.threadID]->complexity = 0;
			m_runningTaskQueue.SetAt(pos, info);
		}
		m_runningTaskQueue.GetNext(pos);
	}
	csRunning.Unlock();

	WriteLog();
	return 0;
}

LRESULT CRamseyXDlg::OnClrOutdated(WPARAM wParam, LPARAM lParam)
{
	time_t nCurrentTime = time(NULL);
	POSITION pos;

	// Todo queue
	csTodo.Lock();

	pos = m_todoTaskQueue.GetHeadPosition();
	while (pos)
	{
		if (m_todoTaskQueue.GetAt(pos).deadline < nCurrentTime)
		{
			POSITION del = pos;
			m_todoTaskQueue.GetNext(pos);
			m_todoTaskQueue.RemoveAt(del);
			continue;
		}
		m_todoTaskQueue.GetNext(pos);
	}

	csTodo.Unlock();
	
	// Running queue
	csRunning.Lock();

	pos = m_runningTaskQueue.GetHeadPosition();
	while (pos)
	{
		if (m_runningTaskQueue.GetAt(pos).deadline < nCurrentTime)
		{
			POSITION del = pos;
			m_runningTaskQueue.GetNext(pos);
			m_runningTaskQueue.RemoveAt(del);
			continue;
		}
		m_runningTaskQueue.GetNext(pos);
	}

	csRunning.Unlock();
	
	return 0;
}

UINT UpdateBenchmarkProc(LPVOID lpParam)
{
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

	MyCurlWrapper conn;
	// check for update
	if (!conn.StandardOpt("www.ramseyx.org/update_benchmark.php") || !conn.SetPost())
		return 0;

	CCPUInfo cpuinfo;
	char szComputerName[MAX_PATH + 2];
	DWORD dwSize = MAX_PATH;
	::GetComputerNameA(szComputerName, &dwSize);
	CStringA strBenchmark;
	strBenchmark.Format("%.1f", pDlg->m_fTotalVAX_MIPS / (CTime::GetCurrentTime().GetTime() - pDlg->m_tLastBenchmark));
	
	pDlg->m_fTotalVAX_MIPS = 0.0f;
	pDlg->m_tLastBenchmark = CTime::GetCurrentTime().GetTime();

	conn.AddPostField("username", CStringA(pDlg->m_strUsername));
	conn.AddPostField("password", CStringA(pDlg->m_strPassword));
	conn.AddPostField("computer_name", szComputerName);
	conn.AddPostField("cpu_brand", CStringA(cpuinfo.GetBrand()));
	conn.AddPostField("benchmark", strBenchmark);

	conn.Execute();

	return 0;
}
