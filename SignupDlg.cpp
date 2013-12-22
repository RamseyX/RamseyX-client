// SignupDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RamseyX.h"
#include "SignupDlg.h"
#include "afxdialogex.h"
#include "CPUInfo.h"

#define END { pSu->SendMessage(WM_ENABLECTRL, TRUE, IDC_SUBMIT); \
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED); \
	return 0; }

inline BOOL Validate(TCHAR c)
{
	return (c >= _T('a') && c <= _T('z')) || (c >= _T('A') && c <= _T('Z')) || (c >= _T('0') && c <= _T('9')) || c == _T('_');
}

// CSignupDlg 对话框

IMPLEMENT_DYNAMIC(CSignupDlg, CDialogEx)

CSignupDlg::CSignupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSignupDlg::IDD, pParent),
	  m_pSubmitThread(NULL)
{

}

CSignupDlg::~CSignupDlg()
{
}

void CSignupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSignupDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SUBMIT, &CSignupDlg::OnBnClickedSubmit)
	ON_MESSAGE(WM_SEL, &CSignupDlg::OnSel)
	ON_WM_CTLCOLOR()
	//ON_WM_PAINT()
	ON_MESSAGE(WM_ENABLECTRL, &CSignupDlg::OnEnableCtrl)
END_MESSAGE_MAP()


// CSignupDlg 消息处理程序


LRESULT CSignupDlg::OnEnableCtrl(WPARAM wParam, LPARAM lParam)
{
	CWnd *pCtrl = GetDlgItem(lParam);
	if (pCtrl)
		pCtrl->EnableWindow(wParam);
	return 0;
}

void CSignupDlg::OnBnClickedSubmit()
{
	VERIFY(m_pSubmitThread = AfxBeginThread(SmThreadProc, static_cast<LPVOID>(this)));
}

LRESULT CSignupDlg::OnSel(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(lParam)->SetFocus();
	static_cast<CEdit*>(GetDlgItem(lParam))->SetSel(0, GetDlgItem(lParam)->GetWindowTextLength());
	return 0;
}

UINT SmThreadProc(LPVOID lpParam)
{
	CSignupDlg* pSu = static_cast<CSignupDlg*>(lpParam);
	
	pSu->SendMessage(WM_ENABLECTRL, FALSE, IDC_SUBMIT);
	CMenu* pSysMenu = pSu->GetSystemMenu(FALSE);
	VERIFY(pSysMenu);
	pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_DISABLED);

	// 字段格式验证
	CString strUsername, strPassword, strCfPassword, strEmail, strRecommender;
	pSu->GetDlgItem(IDC_SUSR)->GetWindowText(strUsername);
	pSu->GetDlgItem(IDC_SPWD)->GetWindowText(strPassword);
	pSu->GetDlgItem(IDC_SCPWD)->GetWindowText(strCfPassword);
	pSu->GetDlgItem(IDC_SEML)->GetWindowText(strEmail);
	pSu->GetDlgItem(IDC_RECOMMENDER)->GetWindowText(strRecommender);
	strEmail.MakeLower();

	// 用户名验证
	if (strUsername.GetLength() < 1 || strUsername.GetLength() > 16)
	{
		pSu->MessageBox(_T("用户名长度应在1-16位之间。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_SUSR);
		END
	}
	else
		for (int i = 0; i < strUsername.GetLength(); i++)
			if (!Validate(strUsername[i]))
			{
				pSu->MessageBox(_T("用户名只能由大小写英文字母、数字及下划线构成。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
				pSu->SendMessage(WM_SEL, 0, IDC_SUSR);
				END
			}

	// 密码验证
	if (strPassword.GetLength() < 4 || strPassword.GetLength() > 16)
	{
		pSu->MessageBox(_T("密码长度应在4-16位之间。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_SPWD);
		END
	}
	else
		for (int i = 0; i < strPassword.GetLength(); i++)
			if (!Validate(strPassword[i]))
			{
				pSu->MessageBox(_T("密码只能由大小写英文字母、数字及下划线构成。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
				pSu->SendMessage(WM_SEL, 0, IDC_SPWD);
				END
			}

	if (strPassword != strCfPassword)
	{
		pSu->MessageBox(_T("两次密码输入不一致。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_SCPWD);
		END
	}
	// 邮箱验证
	if (strEmail.IsEmpty())
	{
		pSu->MessageBox(_T("Email地址不得为空。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_SEML);
		END
	}
	CAtlRegExp<> re;
	// Only one match group
	re.Parse(_T("{[0-9a-z_\\-]+(\\.[0-9a-z_\\-]+)*@([0-9a-z_\\-]+\\.)+[a-z]+}"));
	CAtlREMatchContext<> mc;
	re.Match(strEmail, &mc);
	
	if (mc.m_uNumGroups == 0)
	{
		pSu->MessageBox(_T("无效的邮箱格式。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_SEML);
		END
	}
	else
	{
		const CAtlREMatchContext<>::RECHAR* lszStart = NULL;
		const CAtlREMatchContext<>::RECHAR* lszEnd = NULL;
		mc.GetMatch(0, &lszStart, &lszEnd);
		ptrdiff_t nLength = lszEnd - lszStart;
		CString strTmp;
		strTmp.Format(_T("%.*s"), nLength, lszStart);

		if (strEmail != strTmp)
		{
			pSu->MessageBox(_T("无效的邮箱格式。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
			pSu->SendMessage(WM_SEL, 0, IDC_SEML);
			END
		}
	}

	// 推荐人验证
	if (!strRecommender.IsEmpty() && (strRecommender.GetLength() < 1 || strRecommender.GetLength() > 16))
	{
		pSu->MessageBox(_T("推荐人用户名长度应在1-16位之间。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_RECOMMENDER);
		END
	}
	else
		for (int i = 0; i < strRecommender.GetLength(); i++)
			if (!Validate(strRecommender[i]))
			{
				pSu->MessageBox(_T("推荐人用户名只能由大小写英文字母、数字及下划线构成。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
				pSu->SendMessage(WM_SEL, 0, IDC_RECOMMENDER);
				END
			}

	pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交中..."));
	
	MyCurlWrapper conn;

	if (!conn.StandardOpt("www.ramseyx.org/sign_up.php") || !conn.SetPost())
	{
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("提交失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		END
	}

	TCHAR lszComputerName[MAX_PATH + 2];
	DWORD dwSize = MAX_PATH;
	::GetComputerName(lszComputerName, &dwSize);
	CString strComputerName(lszComputerName);
	CCPUInfo cpu;
	conn.AddPostField("username", CStringA(strUsername));
	conn.AddPostField("password", CStringA(strPassword));
	conn.AddPostField("email", CStringA(strEmail));
	conn.AddPostField("recommender", CStringA(strRecommender));
	conn.AddPostField("computer_name", CStringA(strComputerName));
	conn.AddPostField("cpu_brand", CStringA(cpu.GetBrand()));
	
	if (!conn.Execute())
	{
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("无法连接到服务器。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		END
	}
	
	switch (conn.GetErrorCode())
	{
	case ERR_SUCCESS:
		break;
	case ERR_CONNECTION_FAILED:
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("无法连接到服务器。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		END
	case ERR_SIGNUP_FAILED:
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("提交失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		END
	case ERR_USER_ALREADY_EXIST:
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("该用户名已存在。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_SUSR);
		END
	case ERR_EMAIL_ALREADY_EXIST:
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("该Email地址已存在。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_SEML);
		END
	case ERR_INVALID_RECOMMENDER:
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("该推荐人不存在。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		pSu->SendMessage(WM_SEL, 0, IDC_RECOMMENDER);
		END
	default:
		pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
		pSu->MessageBox(_T("提交失败。\r\n请稍后重试。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONERROR);
		END
	}

	pSu->GetDlgItem(IDC_SUBMIT)->SetWindowText(_T("提交(&S)"));
	pSu->MessageBox(_T("注册成功。\r\n尊敬的用户 ")
		+ strUsername
		+ _T("，您已成功加入 RamseyX 计划。现在，您可以贡献您宝贵的运算力，")
		_T("登录查看您的个人战绩，上传您的运算数据，以及提高您在计划中的排名啦！"),
		_T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);
	pSu->SendMessage(WM_CLOSE);

	return 0;
}


BOOL CSignupDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}


HBRUSH CSignupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_REQUIRED1:
	case IDC_REQUIRED2:
	case IDC_REQUIRED3:
	case IDC_REQUIRED4:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(237, 28, 36));
		break;
	default:
		break;
	}

	return hbr;
}


/*void CSignupDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	CRect rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect, DLG_BK_COLOR);
	// 不为绘图消息调用 CDialogEx::OnPaint()
}*/
