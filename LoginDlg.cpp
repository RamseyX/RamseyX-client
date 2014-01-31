// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RamseyX.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "RamseyXDlg.h"
#include "SignupDlg.h"
#include "RamseyXUtils.h"
#include <regex>

#define END { pLg->GetDlgItem(IDC_LOGIN)->SetWindowText(_T("立刻上传(&L)")); \
	pLg->SendMessage(WM_ENABLECTRL, TRUE, IDC_LOGIN); \
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED); \
	return 0; }

#define END3 { pDlg->m_bIsLocked = false; \
	pLg->CheckDlgButton(IDC_LOCK, 0); \
	pLg->GetDlgItem(IDC_LOGIN)->SetWindowText(_T("立刻上传(&L)")); \
	pLg->SendMessage(WM_ENABLECTRL, TRUE, IDC_USERNAME); \
	pLg->SendMessage(WM_ENABLECTRL, TRUE, IDC_PASSWORD); \
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED); \
	return 0; }

inline BOOL Validate(TCHAR c)
{
	return (c >= _T('a') && c <= _T('z')) || (c >= _T('A') && c <= _T('Z')) || (c >= _T('0') && c <= _T('9')) || c == _T('_');
}

// CLoginDlg 对话框

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLoginDlg::IDD, pParent),
	  m_pLoginThread(nullptr),
	  m_bIsFirst(true)
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SIGNUP, &CLoginDlg::OnBnClickedSignup)
	ON_BN_CLICKED(IDC_LOGIN, &CLoginDlg::OnBnClickedLogin)
	ON_MESSAGE(WM_SEL, &CLoginDlg::OnSel)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_LOCK, &CLoginDlg::OnBnClickedLock)
	ON_MESSAGE(WM_ENABLECTRL, &CLoginDlg::OnEnableCtrl)
END_MESSAGE_MAP()


// CLoginDlg 消息处理程序


LRESULT CLoginDlg::OnEnableCtrl(WPARAM wParam, LPARAM lParam)
{
	CWnd *pCtrl = GetDlgItem(lParam);
	if (pCtrl)
		pCtrl->EnableWindow(wParam);
	return 0;
}

void CLoginDlg::OnBnClickedSignup()
{
	CSignupDlg dlg;
	dlg.DoModal();
}

void CLoginDlg::OnBnClickedLogin()
{
	VERIFY(m_pLoginThread = AfxBeginThread(LgThreadProc, static_cast<LPVOID>(this)));
}

LRESULT CLoginDlg::OnSel(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(lParam)->SetFocus();
	static_cast<CEdit*>(GetDlgItem(lParam))->SetSel(0, GetDlgItem(lParam)->GetWindowTextLength());
	return 0;
}

UINT ValidateProc(LPVOID lpParam)
{
	CLoginDlg *pLg = static_cast<CLoginDlg *>(lpParam);
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());
	
	pLg->SendMessage(WM_ENABLECTRL, FALSE, IDC_USERNAME);
	pLg->SendMessage(WM_ENABLECTRL, FALSE, IDC_PASSWORD);
	pLg->SendMessage(WM_ENABLECTRL, FALSE, IDC_LOGIN);
	pLg->GetDlgItem(IDC_LOGIN)->SetWindowText(_T("验证中..."));
	CMenu *pSysMenu = pLg->GetSystemMenu(FALSE);
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_DISABLED);
	
	// 验证格式
	CString strUsername, strPassword;
	pLg->GetDlgItem(IDC_USERNAME)->GetWindowText(strUsername);
	pLg->GetDlgItem(IDC_PASSWORD)->GetWindowText(strPassword);
	
	// 过长过短/非法字符
	if (strUsername.GetLength() < 1 || strUsername.GetLength() > 16)
	{
		pLg->MessageBox(_T("用户名长度应在1-16位之间。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pLg->SendMessage(WM_SEL, 0, IDC_USERNAME);
		END3
	}
	else
		for (int i = 0; i < strUsername.GetLength(); ++i)
			if (!Validate(strUsername[i]))
			{
				pLg->MessageBox(_T("用户名只能由大小写英文字母、数字及下划线构成。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
				pLg->SendMessage(WM_SEL, 0, IDC_USERNAME);
				END3
			}

	if (strPassword.GetLength() < 4 || strPassword.GetLength() > 16)
	{
		pLg->MessageBox(_T("密码长度应在4-16位之间。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pLg->SendMessage(WM_SEL, 0, IDC_PASSWORD);
		END3
	}
	else
		for (int i = 0; i < strPassword.GetLength(); ++i)
			if (!Validate(strPassword[i]))
			{
				pLg->MessageBox(_T("密码只能由大小写英文字母、数字及下划线构成。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
				pLg->SendMessage(WM_SEL, 0, IDC_PASSWORD);
				END3
			}

	// Check version
	int P = 0, S = 0, T = 0;
	if (pDlg->m_controller.getVersion(P, S, T) != ERR_SUCCESS)
	{
		pLg->MessageBox(_T("版本验证失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		END3
	}
	if (PRIMARY_VERSION < P || (PRIMARY_VERSION == P && SECONDARY_VERSION < S))
	{
		CString strVersion;
		strVersion.Format(_T("%d.%d.%d"), P, S, T);

		if (IDOK ==
			pLg->MessageBox(_T("您的客户端版本过低，无法上传，请更新至最新版程序。本地日志将会被保留。点击确定立刻更新至 ") +
			strVersion + _T("。"), _T("RamseyX 运算客户端"), MB_OKCANCEL | MB_ICONINFORMATION))
		{
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

			pLg->SendMessage(WM_CLOSE);
			pDlg->m_bIsUpdating = true;
			pDlg->SendMessage(WM_CLOSE);

			return 0;
		}
		else
				END3
	}

	// Validate username and password
	switch (pDlg->m_controller.validateUser(
		LPCTSTR(pDlg->m_strUsername),
		LPCTSTR(pDlg->m_strPassword),
		pDlg->m_uID))
	{
		case ERR_SUCCESS:
			break;
		case ERR_CONNECTION_FAILED:
			pLg->MessageBox(_T("无法连接到服务器。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			END3
		case ERR_WRONG_USR_PWD:
			pLg->MessageBox(_T("用户名或密码错误。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			pLg->SendMessage(WM_SEL, 0, IDC_USERNAME);
			END3
		default:
			pLg->MessageBox(_T("验证失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			END3
	}

	pDlg->m_bIsLocked = true;
	pDlg->SetTimer(4, 60000, nullptr);
	pDlg->GetDlgItem(IDC_ME_CAPTION)->SetWindowText(strUsername + _T(" 的战绩："));
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_ME);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_GET_NEW_TASK);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTOUPLOAD);
	pDlg->SendMessage(WM_ENABLECTRL, TRUE, IDC_DISABLE_AUTODOWNLOAD);
	pDlg->GetDlgItem(IDC_GET_NEW_TASK_TIP)->SetWindowText(_T(""));
	pLg->SendMessage(WM_ENABLECTRL, TRUE, IDC_LOGIN);
	pLg->GetDlgItem(IDC_LOGIN)->SetWindowText(_T("立刻上传(&L)"));
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
	pDlg->m_strUsername = strUsername;
	pDlg->m_strPassword = strPassword;
	pDlg->StoreAccount();
	pDlg->OnBnClickedRefreshRank();
	pLg->MessageBox(_T("账户设置成功！"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
	return 0;
}

UINT LgThreadProc(LPVOID lpParam)
{
	CLoginDlg *pLg = static_cast<CLoginDlg *>(lpParam);
	CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

	pLg->SendMessage(WM_ENABLECTRL, FALSE, IDC_LOGIN);
	CMenu *pSysMenu = pLg->GetSystemMenu(FALSE);
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_DISABLED);

	// 验证格式
	CString strUsername, strPassword;
	pLg->GetDlgItem(IDC_USERNAME)->GetWindowText(strUsername);
	pLg->GetDlgItem(IDC_PASSWORD)->GetWindowText(strPassword);
	
	// 过长过短/非法字符
	if (strUsername.GetLength() < 1 || strUsername.GetLength() > 16)
	{
		pLg->MessageBox(_T("用户名长度应在1-16位之间。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pLg->SendMessage(WM_SEL, 0, IDC_USERNAME);
		END
	}
	else
		for (int i = 0; i < strUsername.GetLength(); ++i)
			if (!Validate(strUsername[i]))
			{
				pLg->MessageBox(_T("用户名只能由大小写英文字母、数字及下划线构成。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
				pLg->SendMessage(WM_SEL, 0, IDC_USERNAME);
				END
			}

	if (strPassword.GetLength() < 4 || strPassword.GetLength() > 16)
	{
		pLg->MessageBox(_T("密码长度应在4-16位之间。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pLg->SendMessage(WM_SEL, 0, IDC_PASSWORD);
		END
	}
	else
		for (int i = 0; i < strPassword.GetLength(); ++i)
			if (!Validate(strPassword[i]))
			{
				pLg->MessageBox(_T("密码只能由大小写英文字母、数字及下划线构成。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
				pLg->SendMessage(WM_SEL, 0, IDC_PASSWORD);
				END
			}

	pLg->GetDlgItem(IDC_LOGIN)->SetWindowText(_T("上传中..."));

	// Check for update
	int P = 0, S = 0, T = 0;
	if (pDlg->m_controller.getVersion(P, S, T) != ERR_SUCCESS)
	{
		pLg->MessageBox(_T("版本验证失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		END
	}
	if (PRIMARY_VERSION < P || (PRIMARY_VERSION == P && SECONDARY_VERSION < S))
	{
		CString strVersion;
		strVersion.Format(_T("%d.%d.%d"), P, S, T);

		if (IDOK ==
			pLg->MessageBox(_T("您的客户端版本过低，无法上传，请更新至最新版程序。点击确定立刻更新至 ") +
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

			pLg->SendMessage(WM_CLOSE);
			pDlg->m_bIsUpdating = true;
			pDlg->SendMessage(WM_CLOSE);

			return 0;
		}
		else
			END
	}

	// Upload
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
		case ERR_NO_COMPLETED_TASK:
			pLg->MessageBox(_T("目前没有可以上传的数据。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
			END
		case ERR_CONNECTION_FAILED:
			pLg->MessageBox(_T("无法连接到服务器。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			END
		case ERR_GET_FAILED:
			pLg->MessageBox(_T("获取用户信息失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			END
		case ERR_LOGIN_FAILED:
			pLg->MessageBox(_T("登录失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			END
		case ERR_WRONG_USR_PWD:
			pLg->MessageBox(_T("用户名或密码错误。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			END
		default:
			pLg->MessageBox(_T("上传失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			END
	}
	
	pDlg->SendMessage(WM_THREADOPEN);

	pDlg->SendMessage(WM_UPDATESTATUS);
	pDlg->OnBnClickedRefreshRank();
	pLg->MessageBox(_T("本地数据成功上传至 RamseyX 服务器。您的战绩已刷新。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
	
	END
}

BOOL CLoginDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CLoginDlg::OnClose()
{
	GetDlgItem(IDC_USERNAME)->GetWindowText(static_cast<CRamseyXDlg*>(AfxGetApp()->GetMainWnd())->m_strUsername);
	GetDlgItem(IDC_PASSWORD)->GetWindowText(static_cast<CRamseyXDlg*>(AfxGetApp()->GetMainWnd())->m_strPassword);

	CDialogEx::OnClose();
}


void CLoginDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	if (m_bIsFirst)
	{
		m_bIsFirst = false;
		
		CRamseyXDlg *pDlg = static_cast<CRamseyXDlg*>(AfxGetApp()->GetMainWnd());
		GetDlgItem(IDC_USERNAME)->SetWindowText(pDlg->m_strUsername);
		GetDlgItem(IDC_PASSWORD)->SetWindowText(pDlg->m_strPassword);
		if (pDlg->m_bIsLocked)
		{
			GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_USERNAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
			CheckDlgButton(IDC_LOCK, 1);
		}
		else
			CheckDlgButton(IDC_LOCK, 0);
	}
	// 不为绘图消息调用 CDialogEx::OnPaint()
}

void CLoginDlg::OnBnClickedLock()
{
	if (IsDlgButtonChecked(IDC_LOCK) == 1)
		AfxBeginThread(ValidateProc, static_cast<LPVOID>(this));
	else
	{
		CRamseyXDlg *pDlg = static_cast<CRamseyXDlg *>(AfxGetApp()->GetMainWnd());

		if (!pDlg->m_controller.allListsEmpty() &&
			IDCANCEL ==
			MessageBox(_T("警告：您的计算机上还有未完成或未上传的任务。如果您现在使用不同的账户下载或上传任务，将会使原有的任务全部失效。"),
			_T("RamseyX 运算客户端"), MB_OKCANCEL | MB_ICONEXCLAMATION))
		{
			CheckDlgButton(IDC_LOCK, 1);
			return;
		}
		
		pDlg->GetDlgItem(IDC_ME_CAPTION)->SetWindowText(_T("我的战绩(请在“我的账户”中设置账户)："));
		pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_GET_NEW_TASK);
		pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_DISABLE_AUTOUPLOAD);
		pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_DISABLE_AUTODOWNLOAD);
		pDlg->GetDlgItem(IDC_GET_NEW_TASK_TIP)->SetWindowText(_T("(请先设置账户)"));
		pDlg->m_bIsLocked = false;
		pDlg->KillTimer(4);
		pDlg->StoreAccount();

		pDlg->SendMessage(WM_CLEARMYLIST);
		pDlg->SendMessage(WM_ENABLECTRL, FALSE, IDC_ME);

		GetDlgItem(IDC_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERNAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);
	}
}
